import tkinter as tk
from tkinter import ttk, filedialog, scrolledtext, messagebox
import subprocess
import threading
import os
import shutil
import queue


class CalculatorBuilderGUI:
    """Simple Tkinter GUI to build the C++ calculator into a Windows .exe
    and view the build logs in real time. Auto-detects Visual Studio so
    you do not have to run the Developer Command Prompt manually."""

    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("C++ Calculator -> EXE Builder (Auto VS Detect)")
        self.root.geometry("900x700")
        self.root.minsize(700, 550)

        self.log_queue: queue.Queue = queue.Queue()
        self.building = False
        self.detected_cl: str | None = None
        self.detected_vcvars: str | None = None

        self.create_widgets()
        self.poll_queue()
        self.auto_detect_vs()

    # ------------------------------------------------------------------ UI
    def create_widgets(self):
        # ---- Visual Studio detection bar
        info_frame = ttk.LabelFrame(self.root, text="Visual Studio Detection", padding="10")
        info_frame.pack(fill=tk.X, padx=10, pady=5)

        self.vs_status_var = tk.StringVar(value="Scanning for Visual Studio...")
        ttk.Label(info_frame, textvariable=self.vs_status_var, foreground="blue").grid(
            row=0, column=0, sticky=tk.W
        )
        ttk.Button(info_frame, text="Re-detect", command=self.auto_detect_vs).grid(
            row=0, column=1, padx=10
        )
        ttk.Button(info_frame, text="Open Dev CMD", command=self.open_dev_cmd).grid(
            row=0, column=2, padx=5
        )

        self.cl_path_var = tk.StringVar(value="cl.exe: not found")
        ttk.Label(info_frame, textvariable=self.cl_path_var).grid(
            row=1, column=0, sticky=tk.W, columnspan=3
        )

        # ---- Input row: source file
        input_frame = ttk.Frame(self.root, padding="10")
        input_frame.pack(fill=tk.X)

        ttk.Label(input_frame, text="Source file:").grid(row=0, column=0, sticky=tk.W, pady=5)
        self.source_var = tk.StringVar(value="main.cpp")
        self.source_entry = ttk.Entry(input_frame, textvariable=self.source_var, width=70)
        self.source_entry.grid(row=0, column=1, sticky=tk.EW, padx=5, pady=5)
        ttk.Button(input_frame, text="Browse", command=self.browse_source).grid(row=0, column=2, pady=5)

        # ---- Input row: output exe name
        ttk.Label(input_frame, text="Output EXE:").grid(row=1, column=0, sticky=tk.W, pady=5)
        self.output_var = tk.StringVar(value="calculator.exe")
        self.output_entry = ttk.Entry(input_frame, textvariable=self.output_var, width=70)
        self.output_entry.grid(row=1, column=1, sticky=tk.EW, padx=5, pady=5)

        input_frame.columnconfigure(1, weight=1)

        # ---- Action buttons
        btn_frame = ttk.Frame(self.root, padding="10")
        btn_frame.pack(fill=tk.X)

        self.build_btn = ttk.Button(btn_frame, text="Build EXE", command=self.start_build)
        self.build_btn.pack(side=tk.LEFT, padx=5)

        ttk.Button(btn_frame, text="Clear Logs", command=self.clear_logs).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="Open Folder", command=self.open_folder).pack(side=tk.LEFT, padx=5)

        self.progress = ttk.Progressbar(btn_frame, mode="indeterminate", length=220)
        self.progress.pack(side=tk.RIGHT, padx=5)

        # ---- Log area
        log_frame = ttk.Frame(self.root, padding="10")
        log_frame.pack(fill=tk.BOTH, expand=True)

        ttk.Label(log_frame, text="Build Log:").pack(anchor=tk.W)

        self.log_text = scrolledtext.ScrolledText(
            log_frame, wrap=tk.WORD, height=22, font=("Consolas", 10)
        )
        self.log_text.pack(fill=tk.BOTH, expand=True, pady=5)
        self.log_text.config(state=tk.DISABLED)

        # ---- Status bar
        self.status_var = tk.StringVar(value="Ready")
        self.status_label = ttk.Label(
            self.root, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W
        )
        self.status_label.pack(fill=tk.X, side=tk.BOTTOM)

    # ------------------------------------------------------------------ helpers
    def browse_source(self):
        filename = filedialog.askopenfilename(
            title="Select C++ source file",
            filetypes=[("C++ source", "*.cpp"), ("All files", "*.*")],
        )
        if filename:
            self.source_var.set(filename)

    def log(self, message: str):
        self.log_text.config(state=tk.NORMAL)
        self.log_text.insert(tk.END, message + "\n")
        self.log_text.see(tk.END)
        self.log_text.config(state=tk.DISABLED)

    def clear_logs(self):
        self.log_text.config(state=tk.NORMAL)
        self.log_text.delete("1.0", tk.END)
        self.log_text.config(state=tk.DISABLED)

    def open_folder(self):
        path = self.source_var.get().strip()
        folder = os.path.dirname(os.path.abspath(path)) if path else os.getcwd()
        if os.path.isdir(folder):
            os.startfile(folder)

    def open_dev_cmd(self):
        """Hint for the user if auto-detect fails."""
        messagebox.showinfo(
            "Developer Command Prompt",
            "Open the Start menu and run:\n"
            "Developer Command Prompt for VS 2022\n\n"
            "Then run this GUI from that prompt, or let the auto-detect "
            "find Visual Studio for you.",
        )

    # ------------------------------------------------------------------ VS auto-detect
    def auto_detect_vs(self):
        """Kick off Visual Studio / cl.exe detection in a background thread."""
        self.vs_status_var.set("Detecting Visual Studio...")
        self.cl_path_var.set("cl.exe: searching...")
        self.build_btn.config(state=tk.DISABLED)
        thread = threading.Thread(target=self._detect_vs_thread, daemon=True)
        thread.start()

    def _detect_vs_thread(self):
        # 1. Check PATH first (covers Developer Command Prompt users)
        cl_path = shutil.which("cl")
        if cl_path:
            self.log_queue.put(f"[DETECT] Found cl.exe on PATH: {cl_path}")
            self.detected_cl = cl_path
            self.detected_vcvars = None  # assume environment is already set
            self.root.after(0, self._detection_ui_found, "Found on PATH", cl_path)
            return

        # 2. Search common Visual Studio 2022 installation folders
        vs_roots = [
            r"C:\Program Files\Microsoft Visual Studio\2022",
            r"C:\Program Files (x86)\Microsoft Visual Studio\2022",
        ]
        editions = ["Community", "Professional", "Enterprise", "BuildTools"]

        found = []
        for root in vs_roots:
            if not os.path.isdir(root):
                continue
            for edition in editions:
                vc_dir = os.path.join(root, edition, "VC", "Tools", "MSVC")
                if not os.path.isdir(vc_dir):
                    continue
                versions = [
                    d for d in os.listdir(vc_dir)
                    if os.path.isdir(os.path.join(vc_dir, d))
                ]
                versions.sort(reverse=True)  # latest first
                for ver in versions:
                    host_dir = os.path.join(vc_dir, ver, "bin", "Hostx64", "x64")
                    cl_exe = os.path.join(host_dir, "cl.exe")
                    vcvars = os.path.join(
                        root, edition, "VC", "Auxiliary", "Build", "vcvarsall.bat"
                    )
                    if os.path.isfile(cl_exe):
                        found.append((cl_exe, vcvars, root, edition, ver))
                        break  # only need latest version per edition

        if found:
            cl_exe, vcvars, root, edition, ver = found[0]
            vcvars_ok = vcvars if os.path.isfile(vcvars) else None
            self.log_queue.put(f"[DETECT] Found cl.exe: {cl_exe}")
            self.log_queue.put(f"[DETECT] Visual Studio {edition} {ver}")
            self.log_queue.put(
                f"[DETECT] vcvarsall.bat: {vcvars_ok if vcvars_ok else 'NOT FOUND'}"
            )
            self.detected_cl = cl_exe
            self.detected_vcvars = vcvars_ok
            self.root.after(0, self._detection_ui_found, f"{edition} {ver}", cl_exe)
        else:
            self.log_queue.put("[DETECT] cl.exe not found on PATH or in common VS folders.")
            self.log_queue.put(
                "[DETECT] Install Visual Studio 2022 with C++ build tools, "
                "or run this app from the Developer Command Prompt for VS 2022."
            )
            self.root.after(0, self._detection_ui_not_found)

    def _detection_ui_found(self, status: str, cl_path: str):
        self.vs_status_var.set(f"VS: {status}")
        self.cl_path_var.set(f"cl.exe: {cl_path}")
        self.build_btn.config(state=tk.NORMAL)
        self.status_var.set("Ready - cl.exe detected")

    def _detection_ui_not_found(self):
        self.vs_status_var.set("VS: not found")
        self.cl_path_var.set("cl.exe: not found")
        self.build_btn.config(state=tk.NORMAL)
        self.status_var.set("Ready - cl.exe NOT detected")

    # ------------------------------------------------------------------ build flow
    def poll_queue(self):
        """Pump log messages from the worker thread into the Tkinter text widget."""
        try:
            while True:
                msg = self.log_queue.get_nowait()
                if msg == "__BUILD_DONE__":
                    self.build_finished()
                else:
                    self.log(msg)
        except queue.Empty:
            pass
        self.root.after(100, self.poll_queue)

    def start_build(self):
        if self.building:
            return

        source = self.source_var.get().strip()
        output = self.output_var.get().strip()

        if not source or not os.path.isfile(source):
            messagebox.showerror("Error", "Please select a valid C++ source file.")
            return
        if not output:
            messagebox.showerror("Error", "Please specify an output EXE name.")
            return
        if not self.detected_cl:
            messagebox.showerror(
                "Error",
                "cl.exe was not detected.\n"
                "Click 'Re-detect' or run this app from the Developer Command Prompt for VS 2022.",
            )
            return

        self.building = True
        self.build_btn.config(state=tk.DISABLED)
        self.progress.start()
        self.status_var.set("Building...")
        self.clear_logs()

        thread = threading.Thread(target=self.run_build, args=(source, output), daemon=True)
        thread.start()

    def run_build(self, source: str, output: str):
        cl = self.detected_cl
        vcvars = self.detected_vcvars

        if not cl:
            self.log_queue.put("ERROR: cl.exe not found.")
            self.log_queue.put("Run Auto-Detect or open Developer Command Prompt for VS 2022.")
            self.log_queue.put("__BUILD_DONE__")
            return

        cwd = os.path.dirname(os.path.abspath(source))

        # Build the command. If we found vcvarsall.bat, call it first so cl.exe
        # has the right INCLUDE / LIB / PATH environment variables.
        if vcvars and os.path.isfile(vcvars):
            cmd_str = (
                f'call "{vcvars}" x64 && '
                f'cl /EHsc /O2 /std:c++17 /Fe"{output}" "{source}"'
            )
            cmd = ["cmd", "/c", cmd_str]
            self.log_queue.put(f">>> Using vcvarsall.bat: {vcvars}")
        else:
            cmd = [cl, "/EHsc", "/O2", "/std:c++17", f'/Fe"{output}"', f'"{source}"']
            self.log_queue.put(f">>> Using cl.exe directly: {cl}")

        self.log_queue.put(f">>> Working directory: {cwd}")
        self.log_queue.put("")

        try:
            creationflags = (
                subprocess.CREATE_NO_WINDOW
                if hasattr(subprocess, "CREATE_NO_WINDOW")
                else 0
            )
            proc = subprocess.Popen(
                cmd,
                cwd=cwd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
                creationflags=creationflags,
            )

            for line in proc.stdout:
                self.log_queue.put(line.rstrip("\n"))

            proc.wait()

            if proc.returncode == 0:
                self.log_queue.put("")
                self.log_queue.put(f"BUILD SUCCESS: {output}")
                self.log_queue.put(f"Location: {cwd}")
            else:
                self.log_queue.put("")
                self.log_queue.put(f"BUILD FAILED (exit code {proc.returncode})")

        except FileNotFoundError:
            self.log_queue.put("ERROR: cl.exe not found.")
            self.log_queue.put("Make sure Visual Studio 2022 C++ tools are installed.")
        except Exception as exc:  # noqa: BLE001
            self.log_queue.put(f"ERROR: {exc}")

        self.log_queue.put("__BUILD_DONE__")

    def build_finished(self):
        self.building = False
        self.build_btn.config(state=tk.NORMAL)
        self.progress.stop()
        self.status_var.set("Ready")


def main():
    root = tk.Tk()
    CalculatorBuilderGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()
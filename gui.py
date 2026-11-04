import tkinter as tk
from tkinter import ttk, filedialog, scrolledtext, messagebox
import subprocess
import threading
import os
import shutil
import queue


class CalculatorBuilderGUI:
    """Simple Tkinter GUI to build the C++ calculator into a Windows .exe
    and view the build logs in real time."""

    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("C++ Calculator -> EXE Builder")
        self.root.geometry("850x650")
        self.root.minsize(700, 500)

        self.log_queue: queue.Queue = queue.Queue()
        self.building = False

        self.create_widgets()
        self.poll_queue()

    # ------------------------------------------------------------------ UI
    def create_widgets(self):
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

        self.building = True
        self.build_btn.config(state=tk.DISABLED)
        self.progress.start()
        self.status_var.set("Building...")
        self.clear_logs()

        thread = threading.Thread(target=self.run_build, args=(source, output), daemon=True)
        thread.start()

    def find_cl(self) -> str | None:
        """Return 'cl' if it's on PATH, otherwise None."""
        return "cl" if shutil.which("cl") else None

    def run_build(self, source: str, output: str):
        cl = self.find_cl()
        if not cl:
            self.log_queue.put("ERROR: cl.exe not found on PATH.")
            self.log_queue.put("Run this from the Developer Command Prompt for VS 2022.")
            self.log_queue.put("__BUILD_DONE__")
            return

        cmd = [
            cl,
            "/EHsc",
            "/O2",
            "/std:c++17",
            f"/Fe{output}",
            source,
        ]

        self.log_queue.put(f">>> {' '.join(cmd)}")
        self.log_queue.put("")

        cwd = os.path.dirname(os.path.abspath(source))

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
            self.log_queue.put("Make sure you are inside the Developer Command Prompt for VS 2022.")
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
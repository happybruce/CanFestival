"""
Project Porting Tool, used to copy source files to the target directory for different hardware models.
If you don't select a hardware model, it will only copy the common source files.
"""

import tkinter as tk
from tkinter import ttk, scrolledtext
import shutil
import os


def setup_modern_style(window):
    """配置更现代的 ttk 外观。"""
    style = ttk.Style(window)
    available_themes = style.theme_names()
    if "vista" in available_themes:
        style.theme_use("vista")
    elif "clam" in available_themes:
        style.theme_use("clam")

    # 统一字号与控件间距，让界面更紧凑清晰。
    style.configure("TLabel", font=("Segoe UI", 10))
    style.configure("TButton", font=("Segoe UI", 10), padding=(12, 6))
    style.configure("TEntry", padding=4)
    style.configure("TCombobox", padding=3)
    style.configure("Header.TLabel", font=("Segoe UI", 10, "bold"))


def get_files(folder_path, suffix, exclude_files=None):
    file_list = []
    # 遍历目录下所有条目
    for name in os.listdir(folder_path):
        # 拼接完整路径
        full_path = os.path.join(folder_path, name)
        # 判断：是文件 并且 后缀为.h
        if os.path.isfile(full_path) and full_path.endswith(suffix):
            if exclude_files and name in exclude_files:
                continue
            file_list.append(full_path)
    return file_list

# ===================== 配置区（自行修改源文件夹根目录） =====================
Porting_Files = {}

# 公共源文件夹路径
Porting_Files["include"] = get_files("../include", ".h")
Porting_Files["src"] = get_files("../src", ".c", ["symbols.c"])



# ======================================================================

def do_porting():
    global Porting_Files

    """点击按钮执行复制"""
    selected_model = combo_var.get()
    target_path = entry_target.get().strip()
    log_box.delete(1.0, tk.END)

    # 校验选择
    if not selected_model:
        log_box.insert(tk.END, "Warn: You have not selected a hardware model\n")
    else:
        log_box.insert(tk.END, f"Selected Model: {selected_model}\n")
        Porting_Files[f"include/{selected_model}"] = get_files(f"../include/{selected_model}", ".h")
        Porting_Files[f"drv"] = get_files(f"../drivers/{selected_model}", ".c")

    # 校验目标路径
    if not target_path:
        log_box.insert(tk.END, "Error: Please input destination directory\n")
        return

    log_box.insert(tk.END, f"Destination Path: {target_path}\n\n")

    # 创建目标文件夹
    try:
        os.makedirs(target_path, exist_ok=True)
    except Exception as e:
        log_box.insert(tk.END, f"Error creating destination folder: {str(e)}\n")
        return

    for key, file_list in Porting_Files.items():
        # 目标子文件夹路径
        subfolder_path = os.path.join(target_path, key)
        try:
            os.makedirs(subfolder_path, exist_ok=True)
        except Exception as e:
            log_box.insert(tk.END, f"Error creating subfolder '{subfolder_path}': {str(e)}\n")
            continue

        for file in file_list:
            try:
                shutil.copy(file, subfolder_path)
                log_box.insert(tk.END, f"Copied: {file} -> {subfolder_path}\n")
            except Exception as e:
                log_box.insert(tk.END, f"Failed to copy {file}: {str(e)}\n")


# 主窗口创建
root = tk.Tk()
root.title("Project Porting Tool")
root.geometry("520x320")
root.resizable(False, False)
setup_modern_style(root)

# 变量绑定
combo_var = tk.StringVar()

# 第一行：型号选择
main_frame = ttk.Frame(root, padding=(16, 14, 16, 10))
main_frame.pack(fill=tk.BOTH, expand=True)

frame1 = ttk.Frame(main_frame, padding=(0, 0, 0, 8))
frame1.pack(fill=tk.X)
ttk.Label(frame1, text="Hardware Model:", style="Header.TLabel").pack(side=tk.LEFT)
combo = ttk.Combobox(
    frame1,
    textvariable=combo_var,
    values=["cm0", "cm0+", "cm3", "cm4"],
    width=12,
    state="readonly"
)
combo.pack(side=tk.LEFT, padx=10)

# 第二行：目标目录输入
frame2 = ttk.Frame(main_frame, padding=(0, 0, 0, 10))
frame2.pack(fill=tk.X)
ttk.Label(frame2, text="Destination Dir:", style="Header.TLabel").pack(side=tk.LEFT)
entry_target = ttk.Entry(frame2, width=42)
entry_target.pack(side=tk.LEFT, padx=10)

# 第三行：按钮
frame3 = ttk.Frame(main_frame, padding=(0, 0, 0, 10))
frame3.pack()
btn_port = ttk.Button(
    frame3,
    text="Porting",
    command=do_porting,
    width=14
)
btn_port.pack()

# 日志滚动文本框
ttk.Label(main_frame, text="Log Output:", style="Header.TLabel").pack(anchor="w", pady=(0, 4))
log_box = scrolledtext.ScrolledText(
    main_frame,
    width=65,
    height=8,
    font=("Consolas", 9),
    bd=1,
    relief="solid"
)
log_box.pack(fill=tk.BOTH, expand=True)

root.mainloop()


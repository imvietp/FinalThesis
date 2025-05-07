import tkinter as tk
from tkinter import filedialog
import re  # Import the re module

def open_old_file():
    old_file_path = filedialog.askopenfilename(title="Select Old File")
    if old_file_path:
        old_file_entry.delete(0, tk.END)
        old_file_entry.insert(0, old_file_path)

def open_new_file():
    new_file_path = filedialog.askopenfilename(title="Select New File")
    if new_file_path:
        new_file_entry.delete(0, tk.END)
        new_file_entry.insert(0, new_file_path)

def merge_files():
    old_file_path = old_file_entry.get()
    new_file_path = new_file_entry.get()
    if old_file_path and new_file_path:
        with open(old_file_path, 'r') as old_file:
            old_content = old_file.read()

        with open(new_file_path, 'r') as new_file:
            new_content = new_file.read()

        # Extract elements from the old and new files
        old_elements = re.findall(r"'(.+?)'\{([^}]+)\}", old_content)
        new_elements = re.findall(r"'(.+?)'\{([^}]+)\}", new_content)

        # Create a dictionary to store mappings from new elements to old elements
        element_mapping = {}
        for old_name, old_info in old_elements:
            for new_name, new_info in new_elements:
                if new_name == old_name:
                    element_mapping[new_name] = old_info

        # Generate the merged content
        merged_content = ""
        for new_name, new_info in new_elements:
            old_info = element_mapping.get(new_name, new_info)
            merged_content += f"'{new_name}'{{{old_info}}}\n"

        # Save the merged content to a new file
        output_file_path = filedialog.asksaveasfilename(defaultextension=".txt", filetypes=[("Text files", "*.txt")])
        if output_file_path:
            with open(output_file_path, 'w') as output_file:
                output_file.write(merged_content)

            tk.messagebox.showinfo("Success", "Merged file saved successfully!")

# Create main window
root = tk.Tk()
root.title("File Merger")

# Create input fields
old_file_label = tk.Label(root, text="Old File:")
old_file_label.grid(row=0, column=0, padx=5, pady=5)
old_file_entry = tk.Entry(root, width=50)
old_file_entry.grid(row=0, column=1, padx=5, pady=5)
old_file_button = tk.Button(root, text="Open", command=open_old_file)
old_file_button.grid(row=0, column=2, padx=5, pady=5)

new_file_label = tk.Label(root, text="New File:")
new_file_label.grid(row=1, column=0, padx=5, pady=5)
new_file_entry = tk.Entry(root, width=50)
new_file_entry.grid(row=1, column=1, padx=5, pady=5)
new_file_button = tk.Button(root, text="Open", command=open_new_file)
new_file_button.grid(row=1, column=2, padx=5, pady=5)

# Button to merge files and generate a new file
merge_button = tk.Button(root, text="Merge Files", command=merge_files)
merge_button.grid(row=2, column=1, padx=5, pady=5)

root.mainloop()

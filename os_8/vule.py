import os

folder_path = '.'  # Replace with the path to your folder

output_file = 'prompt.txt'

with open(output_file, 'w',encoding='utf-8') as f:
    for file_name in os.listdir(folder_path):
        if file_name.endswith('.c'):
            file_path = os.path.join(folder_path, file_name)
            f.write(f'File: {file_name}\n')
            with open(file_path, 'r', encoding='utf-8') as file:
                contents = file.read()
                f.write(contents)
            f.write('\n\n')
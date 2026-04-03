
import os

final = "";
for file in os.listdir("./"):
    if file.endswith(".png"):
        s = f"""- name: {os.path.splitext(file)[0]}
        palette: global_palette
        style: rlet
        images:
            - fs/{file}
        """
        final = final+f"""
        type: appvar
        name: {os.path.splitext(file)[0].upper()}
        compress: zx0
        converts:
            - {os.path.splitext(file)[0]}
        """
        print(s)
input()
print(final);
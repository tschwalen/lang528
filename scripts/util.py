import subprocess

def get_project_root_directory():
    # assumes that git is installed
    cmd="git rev-parse --show-toplevel"
    process = subprocess.Popen(
        cmd.split(),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    stdout, _ = process.communicate()
    return stdout.decode("utf-8").strip()

def printred():
    print(f"\033[31m{content}\033[0m") 

def printgreen(content: str):
    print(f"\033[32m{content}\033[0m") 

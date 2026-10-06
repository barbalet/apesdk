import ctypes

def main():
    mylib = ctypes.CDLL('./apesim.so')
    
    mylib.python_init()
    
    while mylib.python_quit_check() == 1:
        try:
            user_input = input("")
        except EOFError:
            break
        mylib.python_check_string.argtypes = [ctypes.c_char_p]
        mylib.python_check_string.restype = None
        buf = ctypes.create_string_buffer(user_input.encode("utf-8"))  # makes a writable char[]
        mylib.python_check_string(buf)
    mylib.python_close()
    
if __name__ == "__main__":
    main()

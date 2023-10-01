import sys

untranslations = {
    'D': '.',
    'L': '(',
    'R': ')',
    'C': ',',
    'o': '[',
    'c': ']',
    'A': '&',
    'P': '*',
    'T': '~',
    'S': ' ',
    'Q': '"',
    'e': 'e',
    'h': '#',
    'd': '$',
    'p': '%',
    'q': "'",
    'm': '-',
    's': '/',
    '0': ':',
    '1': ';',
    '2': '<',
    'E': '=',
    'g': '>',
    'b': '\\',
    '3': '^',
    '4': '`',
    '5': '{',
    '6': '|',
    '7': '}',
    '8': '?',
    '9': '+',
    '_': '_'
}


def untranslate(name):
    result = ""
    first_arg = True
    i = 0
    while i < len(name):
        c = name[i]
        if c == '_':
            c2 = name[i + 1]
            if c2 == 'a':
                if first_arg:
                    result += " "
                    first_arg = False
                else:
                    result += ", "
            elif c2 == 'r':
                result += " => "
            else:
                result += untranslations[c2]
                # print(f"{c2} --> {untranslations[c2]}")
            i += 1
        else:
            result += c
        i += 1
    return result


first = True
for arg in sys.argv:
    if not first:
        print(f"{arg} --> {untranslate(arg)}")
    else:
        first = False

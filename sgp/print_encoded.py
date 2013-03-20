#/usr/bin/evn python3

def pr(text, encoding):
    beginning = "{} ({}): ".format(encoding, text)
    if encoding == "utf-8":
        print(beginning + ", ".join(["0x{:02x}".format(int(x)) for x in text.encode(encoding)]))
    elif encoding == "utf-16be":
        encoded = text.encode(encoding)
        words = []
        while len(encoded) > 0:
            words.append(int(encoded[0])*256 + int(encoded[1]))
            encoded = encoded[2:]
        print(beginning + ", ".join(["0x{:04x}".format(x) for x in words]))
    elif encoding == "utf-32be":
        encoded = text.encode(encoding)
        words = []
        while len(encoded) > 0:
            words.append(((int(encoded[0])*256 + int(encoded[1]))*256 + int(encoded[2]))*256 + int(encoded[3]))
            encoded = encoded[4:]
        print(beginning + ", ".join(["0x{:08x}".format(x) for x in words]))


# English word "test" (тест)
print("-----------------------")
pr("test", "utf-8")
pr("test", "utf-16be")
pr("test", "utf-32be")

# Russian word "test" (тест)
print("-----------------------")
pr("тест", "utf-8")
pr("тест", "utf-16be")
pr("тест", "utf-32be")

# Chinese (simplified) word "test" (测试)
print("-----------------------")
pr("测试", "utf-8")
pr("测试", "utf-16be")
pr("测试", "utf-32be")

# Greek word "test" (δοκιμή)
pr("δοκιμή", "utf-8")
pr("δοκιμή", "utf-16be")
pr("δοκιμή", "utf-32be")

import sys

def main():
    for i, line in enumerate(sys.stdin):
        parts = line.strip().split()
        if len(parts) != 2:
            print(f"Line {i} malformed")
            continue

        expected_valid = parts[0] == '1'
        try:
            raw = bytes.fromhex(parts[1])
        except ValueError:
            print(f"Invalid hex at line {i}")
            continue

        try:
            raw.decode('utf-8')
            python_valid = True
            # print("valid UTF8")
        except UnicodeDecodeError:
            python_valid = False
            # print("invalid UTF8")


        if expected_valid != python_valid:
            print(f"Mismatch on line {i}:")
            print(f"  Bytes: {raw}")
            print(f"  C says: {expected_valid}")
            print(f"  Python says: {python_valid}")
            sys.exit(1)

    print("✅ All fuzz tests matched Python UTF-8 validation.")

if __name__ == "__main__":
    main()

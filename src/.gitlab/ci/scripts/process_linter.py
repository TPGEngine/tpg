#!/usr/bin/env python3
"""
A parser to convert linter output to GitLab Code Quality JSON format.

Usage:
    python3 process_linter.py <input_file> <output_file>

Assumes each line in <input_file> is formatted as:
    path/to/file.cpp:line:column: severity: message

For example:
    src/cpp/example.cpp:10:5: warning: Variable 'x' is unused
    src/cpp/example.cpp:20:8: error: Expected ';' after statement

The script produces a JSON array where each issue looks like:
[
  {
    "description": "Variable 'x' is unused",
    "fingerprint": "generated-hash",
    "severity": "minor",
    "location": {
      "path": "src/cpp/example.cpp",
      "lines": { "begin": 10, "end": 10 }
    }
  },
  ...
]
"""

import sys
import json
import hashlib

def generate_fingerprint(issue):
    """
    Generate a fingerprint based on issue details.
    """
    data = f"{issue['description']}{issue['location']['path']}{issue['location']['lines']['begin']}"
    return hashlib.md5(data.encode('utf-8')).hexdigest()

def parse_linter_output(input_file):
    issues = []
    with open(input_file, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            # Split by colon. Expected format:
            # file:line:column: severity: message
            parts = line.split(':')
            if len(parts) < 5:
                # Not enough parts to parse, skip this line.
                continue

            file_path = parts[0]
            try:
                line_number = int(parts[1])
            except ValueError:
                line_number = 1
            # We ignore the column for now.
            severity_indicator = parts[3].strip().lower()
            message = ":".join(parts[4:]).strip()

            # Determine severity. Map "error" to "major" and "warning" to "minor".
            if "error" in severity_indicator:
                severity = "major"
            elif "warning" in severity_indicator:
                severity = "minor"
            else:
                severity = "info"

            issue = {
                "description": message,
                "fingerprint": "",  # will fill later
                "severity": severity,
                "location": {
                    "path": file_path,
                    "lines": {"begin": line_number, "end": line_number}
                }
            }
            issue["fingerprint"] = generate_fingerprint(issue)
            issues.append(issue)
    return issues

def main():
    if len(sys.argv) != 3:
        print("Usage: python3 process_linter.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    issues = parse_linter_output(input_file)

    with open(output_file, 'w') as out:
        json.dump(issues, out, indent=2)
    print(f"Code Quality report written to {output_file}")

if __name__ == "__main__":
    main()

#!/usr/bin/env bash
# Runs a sequence of commands through the `myshell` program to exercise test cases.
# This prints headers and the shell output for each test section.

DIR=$(cd "$(dirname "$0")" && pwd)
cd "$DIR"

echo "Building myshell..."
make

echo
echo "================ 1. Basic Commands ================"
./myshell <<'EOF'
ls
echo hello world
pwd
echo a    b    c
exit
EOF

echo
echo "================ 2. Quoted Strings ================"
./myshell <<'EOF'
echo "hello world"
touch "my file.txt"
ls
exit
EOF

echo
echo "================ 3. Built-in Commands ================"
./myshell <<'EOF'
cd /tmp
pwd
cd
pwd
exit
EOF

echo
echo "================ 4. Output Redirection ================"
./myshell <<'EOF'
echo hello > out.txt
cat out.txt
ls -la > listing.txt
cat listing.txt
exit
EOF

echo
echo "================ 5. Append Redirection ================"
./myshell <<'EOF'
echo line1 > log.txt
echo line2 >> log.txt
echo line3 >> log.txt
cat log.txt
exit
EOF

echo
echo "================ 6. Input Redirection ================"
./myshell <<'EOF'
printf 'apple\nbanana\ncherry\n' > fruits.txt
cat < fruits.txt
wc -l < fruits.txt
sort < fruits.txt
exit
EOF

echo
echo "================ 7. Error Handling ================"
./myshell <<'EOF'
nonexistent_command
cd /nonexistent_directory
cat < nonexistent.txt
exit
EOF

echo
echo "Tests finished. Files created during tests:"
ls -1 out.txt listing.txt log.txt fruits.txt "my file.txt" 2>/dev/null || true

echo
echo "You can remove generated files with:"
echo "  rm -f out.txt listing.txt log.txt fruits.txt 'my file.txt'"

#!/bin/sh

cd "$(dirname "$0")"
mkdir -p output
./print < easy.txt > output/easy-out.dot
./serialize < easy.txt | ./deserialize-print > output/easy-serialized.dot

dot -Tpng easy-expected.dot > output/easy-expected.png
dot -Tpng output/easy-out.dot > output/easy-out.png
dot -Tpng output/easy-serialized.dot > output/easy-serialized.png

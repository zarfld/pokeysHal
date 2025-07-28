#!/bin/bash
echo "Testing compilation of pokeys_async.c..."

# Clean up old artifacts
find . -name '*.o' -delete 2>/dev/null || true
find . -name '*.ver' -delete 2>/dev/null || true
find . -name '*.so' ! -name 'libPoKeysHal.so' -delete 2>/dev/null || true

# Test compilation - this will show detailed errors if any
echo "Running halcompile..."
sudo halcompile --install --userspace --extra-link-args="-L/usr/lib -lPoKeysHal" experimental/pokeys_async.c

echo "Compilation test completed."

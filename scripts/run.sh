# dyld injection via env var
DYLD_FORCE_FLAT_NAMESPACE=1 DYLD_INSERT_LIBRARIES=bin/inject.dylib ../"$1"

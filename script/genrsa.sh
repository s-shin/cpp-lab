#!/bin/bash
set -eu

: ${KEY_DIR:=misc/keys}
: ${PASSPHRASE:=}

name="$1"

if [[ -z "$PASSPHRASE" ]]; then
  PASSPHRASE="$(openssl rand -base64 12 | fold -w 15 | head -1)"
fi

passphrase_path="${KEY_DIR}/${name}_passphrase"
private_key_path="${KEY_DIR}/${name}_private.pem"
public_key_path="${KEY_DIR}/${name}_public.pem"

echo "$PASSPHRASE" > "$passphrase_path"
openssl genrsa -aes256 -out "$private_key_path" -passout "pass:${PASSPHRASE}" 2024
openssl rsa -in "$private_key_path" -passin "pass:${PASSPHRASE}" -pubout -out "$public_key_path"

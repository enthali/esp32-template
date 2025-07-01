#!/usr/bin/env python3
"""
ESP32 Certificate Generator
Generates self-signed certificates for HTTPS server without requiring OpenSSL binary
"""

try:
    from cryptography import x509
    from cryptography.x509.oid import NameOID
    from cryptography.hazmat.primitives import hashes, serialization
    from cryptography.hazmat.primitives.asymmetric import rsa
    from ipaddress import IPv4Address
    import datetime
    import sys
    import os

    def generate_certificate(cert_file="server_cert.pem", key_file="server_key.pem", validity_days=9125):
        """Generate self-signed certificate and private key"""
        
        # Generate private key
        private_key = rsa.generate_private_key(
            public_exponent=65537,
            key_size=2048,
        )
        
        # Generate certificate
        subject = issuer = x509.Name([
            x509.NameAttribute(NameOID.COMMON_NAME, "ESP32-Distance-Sensor"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, "ESP32 Distance Project"),
            x509.NameAttribute(NameOID.ORGANIZATIONAL_UNIT_NAME, "IoT Device"),
        ])
        
        cert = x509.CertificateBuilder().subject_name(
            subject
        ).issuer_name(
            issuer
        ).public_key(
            private_key.public_key()
        ).serial_number(
            x509.random_serial_number()
        ).not_valid_before(
            datetime.datetime.utcnow()
        ).not_valid_after(
            datetime.datetime.utcnow() + datetime.timedelta(days=validity_days)
        ).add_extension(
            x509.SubjectAlternativeName([
                x509.DNSName("esp32-distance-sensor.local"),
                x509.IPAddress(IPv4Address("192.168.4.1")),
            ]),
            critical=False,
        ).sign(private_key, hashes.SHA256())
        
        # Write private key
        with open(key_file, "wb") as f:
            f.write(private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.PKCS8,
                encryption_algorithm=serialization.NoEncryption()
            ))
        
        # Write certificate
        with open(cert_file, "wb") as f:
            f.write(cert.public_bytes(serialization.Encoding.PEM))
        
        print(f"Certificate generated: {cert_file}")
        print(f"Private key generated: {key_file}")
        print(f"Validity: {validity_days} days (25 years)")

    if __name__ == "__main__":
        generate_certificate()

except ImportError:
    print("Error: cryptography library not available")
    print("Install with: pip install cryptography")
    print("Or use OpenSSL instead:")
    print('openssl req -x509 -newkey rsa:2048 -keyout server_key.pem -out server_cert.pem -days 9125 -nodes -subj "/CN=ESP32-Distance-Sensor"')
    sys.exit(1)
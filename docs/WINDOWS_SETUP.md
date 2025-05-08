# Panduan Pengaturan dan Kompilasi di Windows

Dokumen ini menjelaskan cara mengatur lingkungan pengembangan dan melakukan kompilasi project di sistem operasi Windows.

## Prasyarat

Untuk membangun project di Windows, Anda memerlukan beberapa komponen:

1. **MinGW-w64** - Kompiler C++ untuk Windows
2. **OpenSSL** - Library untuk enkripsi
3. **CMake** (opsional) - Build system
4. **Git** - Version control

## Instalasi Komponen

### 1. Instalasi MinGW-w64

MinGW-w64 menyediakan GCC (kompiler C/C++) untuk Windows.

1. **Download installer** dari [https://sourceforge.net/projects/mingw-w64/](https://sourceforge.net/projects/mingw-w64/)
2. **Jalankan installer** dan pilih opsi:
   - Version: terbaru (misalnya 8.1.0)
   - Architecture: x86_64 (untuk sistem 64-bit)
   - Threads: posix
   - Exception: seh
   - Build revision: terbaru
3. **Tambahkan MinGW ke PATH**:
   - Klik kanan "This PC" atau "My Computer"
   - Pilih "Properties"
   - Klik "Advanced system settings"
   - Klik "Environment Variables"
   - Edit variabel "Path" dan tambahkan path instalasi MinGW (contoh: `C:\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin`)

### 2. Instalasi OpenSSL

1. **Download OpenSSL untuk Windows** dari [https://slproweb.com/products/Win32OpenSSL.html](https://slproweb.com/products/Win32OpenSSL.html)
   - Pilih versi "Win64 OpenSSL v1.1.1k" atau yang lebih baru (versi terbaru direkomendasikan)
2. **Jalankan installer**
   - Pilih direktori instalasi (default: `C:\Program Files\OpenSSL-Win64`)
   - Pilih untuk menyalin file DLL ke direktori sistem Windows
3. **Tambahkan OpenSSL ke PATH**:
   - Tambahkan `C:\Program Files\OpenSSL-Win64\bin` ke variabel PATH

### 3. Instalasi CMake (Opsional)

1. **Download installer** dari [https://cmake.org/download/](https://cmake.org/download/)
2. **Jalankan installer** dan pilih opsi "Add CMake to the system PATH"

### 4. Instalasi Git

1. **Download installer** dari [https://git-scm.com/download/win](https://git-scm.com/download/win)
2. **Jalankan installer** dengan opsi default

## Verifikasi Instalasi

Buka Command Prompt atau PowerShell baru dan verifikasi bahwa semua komponen terinstal dengan benar:

```cmd
g++ --version
openssl version
cmake --version (jika diinstal)
git --version
```

## Mengkompilasi Project

Ada dua cara untuk mengkompilasi project ini di Windows:

### Metode 1: Menggunakan Makefile dengan MinGW

1. **Buka PowerShell atau Command Prompt**
2. **Navigasikan ke direktori project**:
   ```cmd
   cd path\to\project
   ```
3. **Buat direktori bin** jika belum ada:
   ```cmd
   mkdir bin
   ```
4. **Kompilasi dengan Make**:

   ```cmd
   mingw32-make windows
   ```

   Jika perintah di atas tidak berfungsi, coba gunakan:

   ```cmd
   mingw32-make.exe windows
   ```

### Metode 2: Kompilasi Manual

Jika Anda mengalami masalah dengan Makefile, Anda dapat mengkompilasi secara manual:

1. **Kompilasi server**:

   ```cmd
   g++ -Wall -std=c++17 -D_WIN32 server/src/*.cpp common/src/*.cpp -o bin/server.exe -lpthread -lcrypto -lssl -lws2_32
   ```

2. **Kompilasi client**:
   ```cmd
   g++ -Wall -std=c++17 -D_WIN32 client/src/*.cpp common/src/*.cpp -o bin/client.exe -lpthread -lcrypto -lssl -lws2_32 -lgdi32 -lgdiplus -luserenv
   ```

## Troubleshooting

### Error: "Cannot find -lcrypto" atau "Cannot find -lssl"

Pastikan OpenSSL terinstal dengan benar dan path ke library OpenSSL ada di variabel PATH. Anda mungkin perlu menentukan path secara manual dengan flag `-L`:

```cmd
g++ ... -L"C:\Program Files\OpenSSL-Win64\lib" -I"C:\Program Files\OpenSSL-Win64\include" ...
```

### Error: "winsock2.h: No such file or directory"

Pastikan Anda mengkompilasi dengan flag `-D_WIN32` untuk mengaktifkan kode khusus Windows.

### Error: Missing DLL ketika menjalankan executable

Copy file DLL berikut ke direktori yang sama dengan file executable (.exe):

- libcrypto-1_1-x64.dll
- libssl-1_1-x64.dll
- libgcc_s_seh-1.dll
- libstdc++-6.dll
- libwinpthread-1.dll

File-file DLL ini biasanya dapat ditemukan di direktori bin dari instalasi MinGW dan OpenSSL.

## Menjalankan Aplikasi

1. **Server**:

   ```cmd
   bin\server.exe
   ```

2. **Client**:
   ```cmd
   bin\client.exe -s <SERVER_IP>
   ```

## Catatan Tambahan

- Pastikan firewall mengizinkan aplikasi client dan server untuk komunikasi jaringan
- Untuk pengembangan, pertimbangkan menggunakan IDE seperti Visual Studio Code dengan ekstensi C/C++
- Project ini dapat dijalankan dengan hak administrator untuk beberapa fitur (misalnya keylogger dan persistence)

# Panduan Pengaturan GitHub

Dokumen ini memberikan panduan langkah demi langkah untuk mengupload dan mengelola project ini di GitHub.

## Persiapan

Sebelum memulai, pastikan Anda telah menginstal Git di sistem Anda. Anda juga memerlukan akun GitHub.

### Instalasi Git

- **Linux (Debian/Ubuntu)**: `sudo apt install git`
- **Linux (Fedora/RHEL/CentOS)**: `sudo dnf install git` atau `sudo yum install git`
- **Windows**: Download dan instal dari [https://git-scm.com/download/win](https://git-scm.com/download/win)
- **macOS**: `brew install git` (menggunakan Homebrew) atau download installer dari [https://git-scm.com/download/mac](https://git-scm.com/download/mac)

### Verifikasi Instalasi Git

```bash
git --version
```

## Membuat Repository di GitHub

1. Login ke akun GitHub Anda di [https://github.com](https://github.com)
2. Klik tombol "+" di sudut kanan atas dan pilih "New repository"
3. Isi formulir pembuatan repository:
   - **Repository name**: Nama untuk project Anda (misalnya "educational-rat")
   - **Description**: Deskripsi pendek tentang project (opsional)
   - **Visibility**: Private (direkomendasikan untuk project jenis ini)
   - **Initialize this repository with**: JANGAN centang opsi ini
4. Klik "Create repository"

## Menginisialisasi Git di Project Lokal

1. Buka terminal dan navigasikan ke direktori project Anda

```bash
cd /path/to/your/project
```

2. Inisialisasi repository Git lokal

```bash
git init
```

3. Tambahkan file ke staging area

```bash
git add .
```

4. Commit perubahan

```bash
git commit -m "Initial commit"
```

5. Hubungkan repository lokal dengan repository GitHub (ganti `username` dan `educational-rat` dengan URL repository Anda)

```bash
git remote add origin https://github.com/username/educational-rat.git
```

6. Push commit ke GitHub

```bash
git push -u origin master
# atau jika menggunakan branch main:
git push -u origin main
```

## Dokumentasi Penting dan Peringatan

### 1. Pastikan .gitignore Sudah Benar

Sebelum push ke GitHub, pastikan file `.gitignore` sudah benar untuk mencegah file yang tidak perlu atau sensitif ikut terunggah.

### 2. Verifikasi Tidak Ada Kredensial Hardcoded

Periksa kembali kode Anda untuk memastikan tidak ada kredensial, API key, atau informasi sensitif lainnya yang di-hardcode dalam kode.

### 3. Dokumentasi Tujuan Pendidikan

Pastikan README.md dan dokumentasi lainnya jelas menyatakan bahwa project ini hanya untuk tujuan pendidikan, termasuk peringatan hukum tentang penggunaan yang tidak sah.

## Manajemen Branch dan Kolaborasi

### Membuat Branch Baru

```bash
git checkout -b nama-fitur
```

### Menggabungkan Branch

```bash
git checkout main
git merge nama-fitur
```

### Update Repository Lokal dari GitHub

```bash
git pull origin main
```

## Mengelola Rilis (Releases)

1. Di halaman repository GitHub, klik "Releases" di sidebar kanan
2. Klik "Create a new release"
3. Isi tag versi (misalnya v1.0.0), judul, dan deskripsi perubahan
4. Upload file binari jika diperlukan
5. Klik "Publish release"

## Mengelola Issues dan Pull Requests

- **Issues**: Gunakan untuk melacak bug, perbaikan, atau fitur baru
- **Pull Requests**: Gunakan untuk mengusulkan perubahan kode dan kolaborasi

## Praktik Keamanan

1. **Jangan** menyimpan kredensial di repository
2. Gunakan repository **private** untuk kode yang sensitif
3. Tetap perbarui dependensi untuk menghindari kerentanan keamanan

## Sumber Belajar Tambahan

- [GitHub Docs](https://docs.github.com/en)
- [Git Cheat Sheet](https://education.github.com/git-cheat-sheet-education.pdf)
- [GitHub Learning Lab](https://lab.github.com/)

# LAPORAN RESMI FP SISTEM OPERASI 2024
## ANGGOTA KELOMPOK IT08

1. Naufal Syafi' Hakim          (5027231022)
2. RM. Novian Malcolm Bayuputra (5027231035)
3. Abid Ubaidillah Adam         (5027231089)

# discorit.c

Discorit adalah aplikasi obrolan sederhana berbasis command-line yang memungkinkan pengguna untuk mendaftar, masuk, dan berinteraksi di berbagai saluran dan ruang obrolan. README ini memberikan petunjuk tentang cara mengkompilasi, menjalankan, dan menggunakan Discorit.

## Prasyarat

- Kompiler GCC
- Pemahaman dasar tentang antarmuka command-line (CLI)
- Server berjalan di localhost pada port 8080

## Kompilasi

Untuk mengkompilasi program Discorit, jalankan perintah berikut di terminal Anda:

```bash
gcc -o discorit discorit.c -lcrypt
```

## Penggunaan

### Mendaftar Pengguna

Untuk mendaftar pengguna baru, gunakan perintah `REGISTER` diikuti dengan nama pengguna dan kata sandi:

```bash
./discorit REGISTER <username> -p <password>
```

Contoh:

```bash
./discorit REGISTER user1 -p password123
```

### Masuk Pengguna

Untuk masuk sebagai pengguna yang sudah ada, gunakan perintah `LOGIN` diikuti dengan nama pengguna dan kata sandi:

```bash
./discorit LOGIN <username> -p <password>
```

Contoh:

```bash
./discorit LOGIN user1 -p password123
```

### Daftar Saluran

Untuk melihat semua saluran yang tersedia, gunakan perintah `LIST CHANNEL`:

```bash
./discorit LIST CHANNEL
```

### Bergabung dengan Saluran

Untuk bergabung dengan saluran yang sudah ada, gunakan perintah `JOIN` diikuti dengan nama saluran dan kunci:

```bash
./discorit JOIN <channel> <key>
```

Contoh:

```bash
./discorit JOIN channel1 key123
```

### Membuat Saluran

Untuk membuat saluran baru, gunakan perintah `CREATE CHANNEL` diikuti dengan nama saluran dan kunci:

```bash
./discorit CREATE CHANNEL <channel> -k <key>
```

Contoh:

```bash
./discorit CREATE CHANNEL channel1 -k key123
```

### Mengedit Saluran

Untuk mengedit nama saluran yang sudah ada, gunakan perintah `EDIT CHANNEL` diikuti dengan nama saluran lama dan nama saluran baru:

```bash
./discorit EDIT CHANNEL <old_channel> TO <new_channel>
```

Contoh:

```bash
./discorit EDIT CHANNEL channel1 TO newchannel1
```

### Menghapus Saluran

Untuk menghapus saluran yang sudah ada, gunakan perintah `DEL CHANNEL` diikuti dengan nama saluran:

```bash
./discorit DEL CHANNEL <channel>
```

Contoh:

```bash
./discorit DEL CHANNEL channel1
```

### Bergabung dengan Ruang

Untuk bergabung dengan ruang dalam saluran, pertama bergabung dengan saluran, kemudian gunakan perintah `JOIN` diikuti dengan nama ruang:

```bash
./discorit JOIN <room>
```

Contoh:

```bash
./discorit JOIN room1
```

### Membuat Ruang

Untuk membuat ruang baru dalam saluran saat ini, gunakan perintah `CREATE ROOM` diikuti dengan nama ruang:

```bash
./discorit CREATE ROOM <room>
```

Contoh:

```bash
./discorit CREATE ROOM room1
```

### Mengedit Ruang

Untuk mengedit nama ruang yang sudah ada, gunakan perintah `EDIT ROOM` diikuti dengan nama ruang lama dan nama ruang baru:

```bash
./discorit EDIT ROOM <old_room> TO <new_room>
```

Contoh:

```bash
./discorit EDIT ROOM room1 TO newroom1
```

### Menghapus Ruang

Untuk menghapus ruang yang sudah ada, gunakan perintah `DEL ROOM` diikuti dengan nama ruang:

```bash
./discorit DEL ROOM <room>
```

Contoh:

```bash
./discorit DEL ROOM room1
```

Untuk menghapus semua ruang dalam saluran saat ini, gunakan:

```bash
./discorit DEL ROOM ALL
```

### Keluar dari Saluran atau Ruang

Untuk keluar dari ruang atau saluran saat ini, gunakan perintah `EXIT`:

```bash
./discorit EXIT
```

### Mengirim Pesan Obrolan

Untuk mengirim pesan obrolan, gunakan perintah `CHAT` diikuti dengan teks pesan:

```bash
./discorit CHAT <text>
```

Contoh:

```bash
./discorit CHAT Halo, dunia!
```

### Melihat Riwayat Obrolan

Untuk melihat riwayat obrolan, gunakan perintah `SEE CHAT`:

```bash
./discorit SEE CHAT
```

# server.c

## Penggunaan

### Menjalankan Server

Untuk menjalankan server DiscorIT, cukup jalankan program yang telah dikompilasi:

```bash
./server
```

Server akan berjalan sebagai daemon dan akan mendengarkan koneksi pada port 8080.

### Perintah yang Didukung

Server DiscorIT mendukung beberapa perintah yang dapat dikirim oleh klien untuk mengelola pengguna, saluran, dan ruang obrolan. Berikut adalah daftar perintah yang didukung:

#### REGISTER

Mendaftarkan pengguna baru.

```text
REGISTER <username> <password>
```

#### LOGIN

Masuk sebagai pengguna yang sudah ada.

```text
LOGIN <username> <password>
```

#### LIST CHANNEL

Melihat daftar semua saluran yang tersedia.

```text
LIST CHANNEL
```

#### JOIN

Bergabung dengan saluran atau ruang obrolan.

```text
JOIN <channel> <key>
JOIN <room>
```

#### CREATE CHANNEL

Membuat saluran baru dengan kunci.

```text
CREATE CHANNEL <channel> -k <key>
```

#### EDIT CHANNEL

Mengedit nama saluran yang ada.

```text
EDIT CHANNEL <old_channel> TO <new_channel>
```

#### DEL CHANNEL

Menghapus saluran yang ada.

```text
DEL CHANNEL <channel>
```

#### CREATE ROOM

Membuat ruang baru di dalam saluran.

```text
CREATE ROOM <channel> <room>
```

#### EDIT ROOM

Mengedit nama ruang yang ada di dalam saluran.

```text
EDIT ROOM <channel> <old_room> TO <new_room>
```

#### DEL ROOM

Menghapus ruang yang ada di dalam saluran. Untuk menghapus semua ruang dalam saluran, gunakan `DEL ROOM ALL`.

```text
DEL ROOM <channel> <room>
DEL ROOM <channel> ALL
```

#### CHAT

Mengirim pesan obrolan ke ruang obrolan saat ini.

```text
CHAT <text>
```

#### SEE CHAT

Melihat riwayat obrolan di ruang obrolan saat ini.

```text
SEE CHAT
```

#### EXIT

Keluar dari ruang obrolan atau saluran saat ini.

```text
EXIT <channel> <room>
EXIT
```

#### REMOVE

Menghapus pengguna dari saluran.

```text
REMOVE <channel> <username>
```

#### BAN

Melarang pengguna dari saluran.

```text
BAN <channel> <username>
```

#### UNBAN

Mengizinkan kembali pengguna yang dilarang dari saluran.

```text
UNBAN <channel> <username>
```

## Struktur Direktori

Server DiscorIT menyimpan data dalam struktur direktori yang terorganisir di dalam folder `DiscorIT`. Berikut adalah struktur direktori dasar:

```
DiscorIT/
├── channels.csv
├── <channel_name>/
│   ├── admin/
│   │   ├── auth.csv
│   │   └── user.log
│   └── <room_name>/
│       └── chat.csv
└── users.csv
```

- `users.csv`: Menyimpan data pengguna terdaftar.
- `channels.csv`: Menyimpan data saluran yang ada.
- `<channel_name>/`: Direktori untuk setiap saluran.
- `<channel_name>/admin/`: Direktori admin untuk setiap saluran.
- `<channel_name>/admin/auth.csv`: Menyimpan data otorisasi pengguna untuk saluran.
- `<channel_name>/admin/user.log`: Menyimpan log pengguna.
- `<channel_name>/<room_name>/`: Direktori untuk setiap ruang dalam saluran.
- `<channel_name>/<room_name>/chat.csv`: Menyimpan riwayat obrolan dalam ruang.

# Monitor.c

## Deskripsi

Program ini adalah sebuah monitor chat yang ditulis dalam bahasa C. Program ini menghubungkan ke server chat pada alamat IP lokal (127.0.0.1) di port 8080 dan memonitor pesan pada saluran dan ruang tertentu.

## Struktur Program

Program ini terdiri dari dua fungsi utama:
1. \`monitor_chat\`: Fungsi ini mengatur koneksi ke server dan memonitor pesan yang diterima.
2. \`main\`: Fungsi ini memproses argumen yang diberikan melalui command line dan memanggil fungsi \`monitor_chat\`.

### Fungsi \`monitor_chat\`

- Membuat socket untuk koneksi.
- Mengatur alamat server dan port.
- Menghubungkan ke server.
- Mengirimkan perintah "MONITOR" ke server dengan parameter \`username\`, \`channel\`, dan \`room\`.
- Menerima dan mencetak pesan dari server secara terus-menerus.

### Fungsi \`main\`

- Memeriksa dan memvalidasi argumen yang diberikan.
- Memanggil fungsi \`monitor_chat\` dengan argumen yang diberikan.

## Cara Kompilasi dan Eksekusi

### Kompilasi

Untuk mengkompilasi program ini, gunakan perintah berikut:

\`\`\`
gcc -o monitor monitor.c
\`\`\`

### Eksekusi

Untuk menjalankan program, gunakan perintah berikut:

\`\`\`
./monitor <username> -channel <channel_name> -room <room_name>
\`\`\`

Contoh:

\`\`\`
./monitor user1 -channel general -room main
\`\`\`

## Keterangan

- \`<username>\`: Nama pengguna yang ingin dimonitor.
- \`<channel_name>\`: Nama saluran yang ingin dimonitor.
- \`<room_name>\`: Nama ruang yang ingin dimonitor.

Program ini akan terus berjalan dan mencetak pesan yang diterima dari server sampai dihentikan secara manual.

## Dependencies

- Program ini membutuhkan library standar C: \`stdio.h\`, \`stdlib.h\`, \`string.h\`, \`unistd.h\`, \`arpa/inet.h\`.

## Catatan

- Pastikan server chat berjalan pada alamat IP lokal (127.0.0.1) di port 8080.
- Program ini belum memiliki mekanisme error handling yang lengkap dan perlu ditingkatkan untuk penggunaan dalam produksi.
- Perintah harus dimasukkan dengan benar untuk memastikan fungsionalitas yang sesuai.


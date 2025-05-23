# Modul 4

## *Soal 1*

Pada soal ini, diberikan sebuah file zip yang berisi 7 file txt yang berisi string hexadecimal yang sangat panjang. File yang berisi string hexadecimal ini, akan diubah menjadi gambar berdasarkan filenya masing-masing.

## 📌 Fitur Utama

### ⬇️ Downloadz Unzip, dan Delete Otomatis

Pada program ini, saya membuat sebuah function untuk mendownload file zip dari url yang telah diberikan. Setelah melakukan download, program akan melakukan unzip terhadap file zip dan akan menghapus file zip tersebut sesuai dengan keinginan soal.

### 🖥️ Conversi Hexadecimal ke Image

Pada function ini, saya mengambil data dari tiap-tiap file dan mengubah hexadecimal menjadi `byte`. Nah byte ini akan otomatis berubah menjadi image yang akan ditampilkan dalam bentuk `.jpeg` dan image nya akan tampil layaknya gambar biasa.

### ❌ Error Handling

Seperti biasa, terdapat error handling jika program tidak dapat melakukan konversi maupun download file zip. Jadi, setiap function akan memiliki error handlingnya tersendiri.

### 📝 Pencatatan Log

Semua konversi dari hexadecimal yang dilakukan program akan tercatat pada file `conversion.log` yang telah dibuat secara otomatis oleh program itu sendiri.

## *Soal 2*

Pada nomor ini, diberikan sebuah file zip bernama `relics` yang berisi file pecahan jpeg yang tiap-tiap filenya berukuran maksimal 1KB. File-file pecahan ini harus digabungkan menjadi satu file image yang akan ditampilkan layaknya gambar biasa.

## 📌 Fitur Utama

### ⬇️ Downloadz Unzip, dan Delete Otomatis

Pada program ini, saya juga membuat sebuah function untuk mendownload file zip dari url yang telah diberikan. Setelah melakukan download, program akan melakukan unzip terhadap file zip dan akan menghapus file zip tersebut sesuai dengan keinginan soal.

### ⚙️ Mounting File

Pada program ini, akan dilakukan mounting program pada sebuah folder bernama `mount_dir` dimana pada folder ini akan menunjukkan file yang telah digabungkan dari semua file pecahan yang ada pada file relics.

### 🔧 Pembatasan Ukuran File pada Folder `relics`

Pada folder `relics`, setiap file dibatasi hanya memiliki maksimal kapasitas 1KB saja, jadi jika terdapat sebuah file yang melebihi 1KB pada folder `mount_dir` maka program akan otomatis memecah file tersebut menjadi beberapa bagian yang masing-masing berukuran 1KB.

### 🧠 Logika Read, Write, Copy, dan Delete File

Pada program ini, terdapat 4 function utama yang mempengaruhi kedua folder, yaitu:

1. `Read` yang akan trigger ketika membuka file dalam `mount_dir`
2. `Write` yang akan trigger ketika membuat sebuah file baru dalam folder `mount_dir`
3. `Copy` yang akan trigger ketika melakukan copy terhadap file yang terdapat pada folder `mount_dir`
4. `Delete` yang akan trigger ketika menghapus file yang ada dalam folder `mount_dir`

Namun, ketika file dalam folder `mount_dir` dihapus, terdapat sebuah syarata yaitu pecahan-pecahan file tersebut yang ada pada folder `relics` akan ikut terhapus juga.

Jika melakukan `Write` atau membuat file baru pada folder `mount_dir`, program akan memecah file tersebut menjadi pecahan-pecahan file yang berukuran maksimal 1KB dan menyimpannya pada folder `relics`. Jika file yang dibuat melebihi 1KB, maka akan terpecah menjadi beberapa pecahan file 1KB dengan format `(nama_file).jpg.000` dan akan bertambah setiap kali dipecah menjadi `(nama_file).jpg.001` dan seterusnya.

### 📝 Pencatatan Log

Jika sebuah function digunakan, maka akan tercatat pada file `activity.log` yang berisi seluruh history penggunaan fitur pada program. Terdapat 4 fitur yang akan dicatat, yaitu `READ`, `WRITE`, `COPY`, dan `DELETE` yang akan ditulis berdasarkan ketentuan format soal.

## Soal 3

## Soal 4

## *Soal*

Pada modul ini, saya diminta untuk membuat sebuah **filesystem virtual** menggunakan FUSE yang merepresentasikan dunia *maimai* dari SEGA. Filesystem ini terdiri dari **7 area (chiho)**, dan setiap chiho memiliki aturan manipulasi file yang berbeda-beda. Tujuan dari soal ini adalah untuk mengimplementasikan aturan tersebut dalam bentuk operasi filesystem seperti `read`, `write`, `create`, `unlink`, dll.

---

## 📌 Fitur Utama

### 📁 Struktur Area (Chiho)

Filesystem terdiri dari 7 direktori/area utama:

```
fuse_dir/
├── starter/
├── metro/
├── dragon/
├── blackrose/
├── heaven/
├── youth/
└── 7sref/
```


Setiap area memiliki perlakuan khusus terhadap file yang disimpan di dalamnya.

---

## *Penjelasan Setiap Area*

### 🅰️ Starter Chiho (Area Pemula)

📝 Semua file disimpan dengan ekstensi tambahan `.mai`, namun saat dilihat di FUSE (mount point), ekstensi ini **disembunyikan**.

🔧 Implementasi:
- Menambahkan `.mai` pada saat `create` dan `unlink`.
- Menghapus ekstensi saat `readdir` untuk tampilan yang bersih.

---

### 🅱️ Metropolis Chiho (World's End)

📝 File disimpan dengan **nama yang telah di-shift** berdasarkan posisi karakter dalam namanya. Misalnya: `ener.txt` disimpan sebagai `eogu.txt`.

🔧 Implementasi:
- Fungsi `shift_file_name()` untuk menggantikan setiap karakter dengan karakter hasil penambahan `(i % 256)`.
- Digunakan saat `create`, `read`, `write`, dan `unlink`.

---

### 🅲️ Dragon Chiho (World Tree)

📝 Isi file akan disimpan dalam bentuk terenkripsi menggunakan algoritma **ROT13**, yaitu penggeseran alfabet sebanyak 13 posisi.

🔧 Implementasi:
- Fungsi `rot_13()` digunakan untuk mengenkripsi dan mendekripsi buffer selama proses `read` dan `write`.

---

### 🅳️ Blackrose Chiho (Black Rose Area)

📝 File disimpan **dalam bentuk biner murni** tanpa enkripsi ataupun encoding tambahan.

🔧 Implementasi:
- File diperlakukan secara default tanpa transformasi nama atau isi.

---

### 🅴️ Heaven Chiho (Tenkai Area)

📝 Semua file disimpan menggunakan enkripsi **AES-256-CBC**, dengan IV (Initialization Vector) yang ditulis di awal file.

🔧 Implementasi:
- Fungsi `aes_encrypt()` dan `aes_decrypt()` digunakan pada saat file dibaca atau ditulis.
- File hasil enkripsi disimpan, sementara plaintext digunakan sebagai file sementara.

---

### 🅵️ Youth Chiho (Skystreet)

📝 Semua file yang disimpan akan dikompres secara otomatis menggunakan **GZIP** untuk menghemat storage.

🔧 Implementasi:
- Fungsi `compress_to_gzip()` digunakan saat `release` file.
- Fungsi `decompress_gzip()` digunakan sebelum `read`.

---

### 🅶️ 7sRef Chiho (Prism Area)

📝 Area ini adalah gateway untuk mengakses seluruh chiho lain dengan sistem penamaan khusus `[area]_[filename]`.

📌 Contoh:

```
/fuse_dir/7sref/starter_guide.txt → /fuse_dir/starter/guide.txt
/fuse_dir/7sref/metro_data.log → /fuse_dir/metro/data.log
```


🔧 Implementasi:
- Fungsi `map_7sref_to_real()` digunakan untuk menerjemahkan path menjadi file asli sesuai area dan nama file.
- Digunakan pada semua operasi seperti `getattr`, `read`, `write`, `create`, dan `unlink`.

---

## 📋 Operasi Filesystem yang Didukung

Berikut adalah daftar fungsi yang diimplementasikan:
- `getattr` → Mengambil atribut file atau folder.
- `readdir` → Membaca isi direktori.
- `open` / `release` → Membuka dan menutup file.
- `read` / `write` → Membaca dan menulis isi file (dengan transformasi).
- `create` / `unlink` → Membuat dan menghapus file.

---

## ⚙️ Cara Kompilasi dan Jalankan

### 1. Kompilasi Program
```
gcc -Wall -o maimai_fs maimai_fs.c `pkg-config fuse3 --cflags --libs` -lcrypto -lz
```

### 2. Jalankan Filesystem
```
./maimai_fs fuse_dir/

```

### 3. Unmount Filesystem
```
fusermount3 -u fuse_dir/

```

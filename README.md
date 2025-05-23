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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PERSIMPANGAN 10
#define MAX_AGENDA 50
#define FILE_LAMPU "lampu.bin"
#define FILE_AGENDA "agenda.txt"

// ==================== ENUM ====================
typedef enum {
    MERAH = 0,  // Bit ke-0
    KUNING = 1, // Bit ke-1
    HIJAU = 2   // Bit ke-2
} WarnaLampu;

// ==================== STRUCT ====================
typedef struct {
    int id;
    char nama[50];
    unsigned char statusLampu; // 3 bit: [Hijau][Kuning][Merah]
} Persimpangan;

typedef struct {
    char namaPersimpangan[50];
    char waktu[20];
    char deskripsi[100];
} AgendaPerawatan;

// ==================== VARIABEL GLOBAL ====================
Persimpangan daftarPersimpangan[MAX_PERSIMPANGAN];
int jumlahPersimpangan = 0;
AgendaPerawatan daftarAgenda[MAX_AGENDA];
int jumlahAgenda = 0;

// ==================== FUNGSI BIT MANIPULATION ====================
void printBinary(unsigned char num, int bits) {
    for (int i = bits - 1; i >= 0; i--) {
        printf("%d", (num >> i) & 1);
    }
}

// Menyalakan lampu menggunakan OR
void nyalakanLampu(Persimpangan *p, WarnaLampu warna) {
    p->statusLampu = p->statusLampu | (1 << warna);
}

// Mematikan lampu menggunakan AND dan NOT
void matikanLampu(Persimpangan *p, WarnaLampu warna) {
    p->statusLampu = p->statusLampu & ~(1 << warna);
}

// Toggle lampu menggunakan XOR
void toggleLampu(Persimpangan *p, WarnaLampu warna) {
    p->statusLampu = p->statusLampu ^ (1 << warna);
}

// Cek status lampu menggunakan AND
int cekLampu(Persimpangan *p, WarnaLampu warna) {
    return (p->statusLampu >> warna) & 1;
}

// ==================== FUNGSI TAMPILAN STATUS LAMPU ====================
void tampilkanStatusLampu(Persimpangan *p) {
    printf("\n=== Status Lampu Persimpangan: %s ===\n", p->nama);
    printf("Representasi Biner: ");
    printBinary(p->statusLampu, 3);
    printf(" (format: [Hijau][Kuning][Merah])\n\n");
    
    printf("Status Detail:\n");
    printf("  Lampu Merah  : %s\n", cekLampu(p, MERAH) ? "ON" : "OFF");
    printf("  Lampu Kuning : %s\n", cekLampu(p, KUNING) ? "ON" : "OFF");
    printf("  Lampu Hijau  : %s\n", cekLampu(p, HIJAU) ? "ON" : "OFF");
}

const char* getNamaWarna(WarnaLampu warna) {
    switch(warna) {
        case MERAH: return "Merah";
        case KUNING: return "Kuning";
        case HIJAU: return "Hijau";
        default: return "Unknown";
    }
}

// ==================== FUNGSI FILE BINER (RANDOM ACCESS) ====================
void simpanStatusLampuBiner() {
    FILE *file = fopen(FILE_LAMPU, "wb");
    if (file == NULL) {
        printf("Error: Tidak dapat membuka file %s\n", FILE_LAMPU);
        return;
    }
    fwrite(&jumlahPersimpangan, sizeof(int), 1, file);
    fwrite(daftarPersimpangan, sizeof(Persimpangan), jumlahPersimpangan, file);
    fclose(file);
    printf("Status lampu berhasil disimpan ke %s\n", FILE_LAMPU);
}

void muatStatusLampuBiner() {
    FILE *file = fopen(FILE_LAMPU, "rb");
    if (file == NULL) {
        printf("Info: File %s tidak ditemukan. Memulai dengan data kosong.\n", FILE_LAMPU);
        return;
    }
    fread(&jumlahPersimpangan, sizeof(int), 1, file);
    fread(daftarPersimpangan, sizeof(Persimpangan), jumlahPersimpangan, file);
    fclose(file);
    printf("Status lampu berhasil dimuat dari %s (%d persimpangan)\n", FILE_LAMPU, jumlahPersimpangan);
}

// Random access: update satu persimpangan berdasarkan index
void updatePersimpanganBiner(int index) {
    if (index < 0 || index >= jumlahPersimpangan) {
        printf("Error: Index tidak valid\n");
        return;
    }
    FILE *file = fopen(FILE_LAMPU, "r+b");
    if (file == NULL) {
        printf("Error: Tidak dapat membuka file\n");
        return;
    }
    // Seek ke posisi persimpangan yang diinginkan
    long offset = sizeof(int) + (index * sizeof(Persimpangan));
    fseek(file, offset, SEEK_SET);
    fwrite(&daftarPersimpangan[index], sizeof(Persimpangan), 1, file);
    fclose(file);
}

// ==================== FUNGSI FILE TEKS (SEQUENTIAL) ====================
void simpanAgendaKeFile() {
    FILE *file = fopen(FILE_AGENDA, "w");
    if (file == NULL) {
        printf("Error: Tidak dapat membuka file %s\n", FILE_AGENDA);
        return;
    }
    fprintf(file, "%d\n", jumlahAgenda);
    for (int i = 0; i < jumlahAgenda; i++) {
        fprintf(file, "%s|%s|%s\n", 
            daftarAgenda[i].namaPersimpangan,
            daftarAgenda[i].waktu,
            daftarAgenda[i].deskripsi);
    }
    fclose(file);
    printf("Agenda berhasil disimpan ke %s\n", FILE_AGENDA);
}

void muatAgendaDariFile() {
    FILE *file = fopen(FILE_AGENDA, "r");
    if (file == NULL) {
        printf("Info: File %s tidak ditemukan. Memulai dengan agenda kosong.\n", FILE_AGENDA);
        return;
    }
    fscanf(file, "%d\n", &jumlahAgenda);
    for (int i = 0; i < jumlahAgenda; i++) {
        fscanf(file, "%[^|]|%[^|]|%[^\n]\n",
            daftarAgenda[i].namaPersimpangan,
            daftarAgenda[i].waktu,
            daftarAgenda[i].deskripsi);
    }
    fclose(file);
    printf("Agenda berhasil dimuat dari %s (%d agenda)\n", FILE_AGENDA, jumlahAgenda);
}

// ==================== FUNGSI MODUL LAMPU ====================
void tambahPersimpangan() {
    if (jumlahPersimpangan >= MAX_PERSIMPANGAN) {
        printf("Error: Jumlah persimpangan maksimal tercapai!\n");
        return;
    }
    Persimpangan *p = &daftarPersimpangan[jumlahPersimpangan];
    p->id = jumlahPersimpangan + 1;
    
    printf("Masukkan nama persimpangan: ");
    getchar();
    fgets(p->nama, 50, stdin);
    p->nama[strcspn(p->nama, "\n")] = 0;
    
    p->statusLampu = 0; // Semua lampu mati
    jumlahPersimpangan++;
    
    printf("Persimpangan '%s' berhasil ditambahkan dengan ID %d\n", p->nama, p->id);
    simpanStatusLampuBiner();
}

void daftarSemuaPersimpangan() {
    if (jumlahPersimpangan == 0) {
        printf("Belum ada persimpangan yang terdaftar.\n");
        return;
    }
    printf("\n========== DAFTAR PERSIMPANGAN ==========\n");
    printf("%-4s %-25s %-10s\n", "ID", "Nama", "Status");
    printf("-----------------------------------------\n");
    for (int i = 0; i < jumlahPersimpangan; i++) {
        printf("%-4d %-25s ", daftarPersimpangan[i].id, daftarPersimpangan[i].nama);
        printBinary(daftarPersimpangan[i].statusLampu, 3);
        printf("\n");
    }
}

void ubahStatusLampu() {
    if (jumlahPersimpangan == 0) {
        printf("Belum ada persimpangan yang terdaftar.\n");
        return;
    }
    daftarSemuaPersimpangan();
    
    int id, pilihan;
    printf("\nMasukkan ID persimpangan: ");
    scanf("%d", &id);
    
    if (id < 1 || id > jumlahPersimpangan) {
        printf("Error: ID tidak valid!\n");
        return;
    }
    
    Persimpangan *p = &daftarPersimpangan[id - 1];
    tampilkanStatusLampu(p);
    
    printf("\nPilih aksi:\n");
    printf("1. Nyalakan lampu\n");
    printf("2. Matikan lampu\n");
    printf("3. Toggle lampu\n");
    printf("Pilihan: ");
    scanf("%d", &pilihan);
    
    int warna;
    printf("Pilih warna (0=Merah, 1=Kuning, 2=Hijau): ");
    scanf("%d", &warna);
    
    if (warna < 0 || warna > 2) {
        printf("Error: Warna tidak valid!\n");
        return;
    }
    
    switch(pilihan) {
        case 1:
            nyalakanLampu(p, (WarnaLampu)warna);
            printf("Lampu %s dinyalakan.\n", getNamaWarna((WarnaLampu)warna));
            break;
        case 2:
            matikanLampu(p, (WarnaLampu)warna);
            printf("Lampu %s dimatikan.\n", getNamaWarna((WarnaLampu)warna));
            break;
        case 3:
            toggleLampu(p, (WarnaLampu)warna);
            printf("Lampu %s di-toggle.\n", getNamaWarna((WarnaLampu)warna));
            break;
        default:
            printf("Pilihan tidak valid!\n");
            return;
    }
    
    tampilkanStatusLampu(p);
    updatePersimpanganBiner(id - 1); // Random access update
}

void menuLampu() {
    int pilihan;
    do {
        printf("\n====== MODUL MANAJEMEN LAMPU LALU LINTAS ======\n");
        printf("1. Tambah Persimpangan\n");
        printf("2. Lihat Daftar Persimpangan\n");
        printf("3. Ubah Status Lampu\n");
        printf("4. Lihat Detail Status Lampu\n");
        printf("5. Simpan ke File\n");
        printf("0. Kembali ke Menu Utama\n");
        printf("Pilihan: ");
        scanf("%d", &pilihan);
        
        switch(pilihan) {
            case 1: tambahPersimpangan(); break;
            case 2: daftarSemuaPersimpangan(); break;
            case 3: ubahStatusLampu(); break;
            case 4: {
                daftarSemuaPersimpangan();
                int id;
                printf("Masukkan ID persimpangan: ");
                scanf("%d", &id);
                if (id >= 1 && id <= jumlahPersimpangan) {
                    tampilkanStatusLampu(&daftarPersimpangan[id-1]);
                } else {
                    printf("ID tidak valid!\n");
                }
                break;
            }
            case 5: simpanStatusLampuBiner(); break;
            case 0: break;
            default: printf("Pilihan tidak valid!\n");
        }
    } while(pilihan != 0);
}

// ==================== FUNGSI MODUL AGENDA ====================
void tambahAgenda() {
    if (jumlahAgenda >= MAX_AGENDA) {
        printf("Error: Jumlah agenda maksimal tercapai!\n");
        return;
    }
    AgendaPerawatan *a = &daftarAgenda[jumlahAgenda];
    getchar();
    
    printf("Masukkan nama persimpangan: ");
    fgets(a->namaPersimpangan, 50, stdin);
    a->namaPersimpangan[strcspn(a->namaPersimpangan, "\n")] = 0;
    
    printf("Masukkan waktu pelaksanaan (DD/MM/YYYY HH:MM): ");
    fgets(a->waktu, 20, stdin);
    a->waktu[strcspn(a->waktu, "\n")] = 0;
    
    printf("Masukkan deskripsi kegiatan: ");
    fgets(a->deskripsi, 100, stdin);
    a->deskripsi[strcspn(a->deskripsi, "\n")] = 0;
    
    jumlahAgenda++;
    printf("Agenda berhasil ditambahkan!\n");
    simpanAgendaKeFile();
}

void tampilkanSemuaAgenda() {
    if (jumlahAgenda == 0) {
        printf("Belum ada agenda perawatan.\n");
        return;
    }
    printf("\n=============== DAFTAR AGENDA PERAWATAN ===============\n");
    printf("%-4s %-20s %-18s %-30s\n", "No", "Persimpangan", "Waktu", "Deskripsi");
    printf("-------------------------------------------------------\n");
    for (int i = 0; i < jumlahAgenda; i++) {
        printf("%-4d %-20s %-18s %-30s\n", 
            i + 1,
            daftarAgenda[i].namaPersimpangan,
            daftarAgenda[i].waktu,
            daftarAgenda[i].deskripsi);
    }
}

void hapusAgenda() {
    if (jumlahAgenda == 0) {
        printf("Belum ada agenda untuk dihapus.\n");
        return;
    }
    tampilkanSemuaAgenda();
    
    int no;
    printf("\nMasukkan nomor agenda yang akan dihapus: ");
    scanf("%d", &no);
    
    if (no < 1 || no > jumlahAgenda) {
        printf("Error: Nomor tidak valid!\n");
        return;
    }
    
    // Geser elemen setelah yang dihapus (sequential deletion)
    for (int i = no - 1; i < jumlahAgenda - 1; i++) {
        daftarAgenda[i] = daftarAgenda[i + 1];
    }
    jumlahAgenda--;
    
    printf("Agenda berhasil dihapus!\n");
    simpanAgendaKeFile();
}

void menuAgenda() {
    int pilihan;
    do {
        printf("\n====== MODUL AGENDA PERAWATAN LAMPU ======\n");
        printf("1. Tambah Agenda\n");
        printf("2. Tampilkan Semua Agenda\n");
        printf("3. Hapus Agenda\n");
        printf("4. Simpan Agenda ke File\n");
        printf("0. Kembali ke Menu Utama\n");
        printf("Pilihan: ");
        scanf("%d", &pilihan);
        
        switch(pilihan) {
            case 1: tambahAgenda(); break;
            case 2: tampilkanSemuaAgenda(); break;
            case 3: hapusAgenda(); break;
            case 4: simpanAgendaKeFile(); break;
            case 0: break;
            default: printf("Pilihan tidak valid!\n");
        }
    } while(pilihan != 0);
}

// ==================== FUNGSI UTAMA ====================
int main() {
    printf("============================================\n");
    printf("   SISTEM MANAJEMEN LAMPU LALU LINTAS\n");
    printf("       DAN AGENDA PERAWATAN\n");
    printf("============================================\n\n");
    
    // Muat data dari file saat program dimulai
    muatStatusLampuBiner();
    muatAgendaDariFile();
    
    int pilihan;
    do {
        printf("\n========== MENU UTAMA ==========\n");
        printf("1. Modul Manajemen Lampu Lalu Lintas\n");
        printf("2. Modul Agenda Perawatan Lampu\n");
        printf("0. Keluar\n");
        printf("Pilihan: ");
        scanf("%d", &pilihan);
        
        switch(pilihan) {
            case 1: menuLampu(); break;
            case 2: menuAgenda(); break;
            case 0: 
                printf("\nMenyimpan semua data...\n");
                simpanStatusLampuBiner();
                simpanAgendaKeFile();
                printf("Terima kasih! Program selesai.\n");
                break;
            default: printf("Pilihan tidak valid!\n");
        }
    } while(pilihan != 0);
    
    return 0;
}
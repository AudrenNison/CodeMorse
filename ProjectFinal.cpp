

#define _CRT_SECURE_NO_WARNINGS


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <algorithm>
#include <cstdint>
#include <regex>
#include <numeric>

const float pi = 3.14159265358979323846;
const int sampleRate = 500; //500 à 44100, plus elle est basse, plus les fichiers seront légers
const int bitDepth = 16;
const float t0 = 0.5; //durée d'un point

std::unordered_map<char, std::string> AlphaMorse;
std::unordered_map<std::string, char> MorseAlpha;





typedef struct  WAV_HEADER
{
    /* RIFF Chunk Descriptor */
    uint8_t         RIFF[4];        // RIFF Header Magic header
    uint32_t        ChunkSize;      // RIFF Chunk Size
    uint8_t         WAVE[4];        // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t         fmt[4];         // FMT header
    uint32_t        Subchunk1Size;  // Size of the fmt chunk
    uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Stereo
    uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
    uint32_t        bytesPerSec;    // bytes per second
    uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
    uint16_t        bitsPerSample;  // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t         Subchunk2ID[4]; // "data"  string
    uint32_t        Subchunk2Size;  // Sampled data length
} wav_hdr;

class SineOscillator {
    float frequency, amplitude, angle = 0.0f, offset=0.0;

public:
    SineOscillator(float freq, float amp) : frequency(freq), amplitude(amp) {
        offset = 2 * pi * frequency / sampleRate;
    }

    float process() {
        auto sample = amplitude * sin(angle);
        angle += offset;
        return sample;
    }
};

//Remplissage du dictionnaire
void fill_AlphaMorse() {
    AlphaMorse.insert({ 'a',". -" });
    AlphaMorse.insert({ 'b',"- . . ." });
    AlphaMorse.insert({ 'c',"- . - ." });
    AlphaMorse.insert({ 'd',"- . ." });
    AlphaMorse.insert({ 'e',"." });
    AlphaMorse.insert({ 'f',". . - ." });
    AlphaMorse.insert({ 'g',"- - ." });
    AlphaMorse.insert({ 'h',". . . ." });
    AlphaMorse.insert({ 'i',". ." });
    AlphaMorse.insert({ 'j',". - - -" });
    AlphaMorse.insert({ 'k',"- . -" });
    AlphaMorse.insert({ 'l',". - . ." });
    AlphaMorse.insert({ 'm',"- -" });
    AlphaMorse.insert({ 'n',"- ." });
    AlphaMorse.insert({ 'o',"- - -" });
    AlphaMorse.insert({ 'p',". - - ." });
    AlphaMorse.insert({ 'q',"- - . -" });
    AlphaMorse.insert({ 'r',". - ." });
    AlphaMorse.insert({ 's',". . ." });
    AlphaMorse.insert({ 't',"-" });
    AlphaMorse.insert({ 'u',". . -" });
    AlphaMorse.insert({ 'v',". . . -" });
    AlphaMorse.insert({ 'w',". - -" });
    AlphaMorse.insert({ 'x',"- . . -" });
    AlphaMorse.insert({ 'y',"- . - -" });
    AlphaMorse.insert({ 'z',"- - . ." });
    AlphaMorse.insert({ ' '," " });
}

void fill_MorseAlpha() {
    MorseAlpha.insert({ ". -",'a'});
    MorseAlpha.insert({ "- . . .",'b'});
    MorseAlpha.insert({ "- . - .",'c'});
    MorseAlpha.insert({ "- . .",'d'});
    MorseAlpha.insert({ ".",'e'});
    MorseAlpha.insert({ ". . - .",'f'});
    MorseAlpha.insert({ "- - .",'g'});
    MorseAlpha.insert({ ". . . .",'h'});
    MorseAlpha.insert({ ". .",'i'});
    MorseAlpha.insert({ ". - - -",'j'});
    MorseAlpha.insert({ "- . -",'k'});
    MorseAlpha.insert({ ". - . .",'l'});
    MorseAlpha.insert({ "- -",'m'});
    MorseAlpha.insert({ "- .",'n'});
    MorseAlpha.insert({ "- - -",'o'});
    MorseAlpha.insert({ ". - - .",'p'});
    MorseAlpha.insert({ "- - . -",'q'});
    MorseAlpha.insert({ ". - .",'r'});
    MorseAlpha.insert({ ". . .",'s'});
    MorseAlpha.insert({ "-",'t'});
    MorseAlpha.insert({ ". . -",'u'});
    MorseAlpha.insert({ ". . . -",'v'});
    MorseAlpha.insert({ ". - -",'w'});
    MorseAlpha.insert({ "- . . -",'x'});
    MorseAlpha.insert({ "- . - -",'y'});
    MorseAlpha.insert({ "- - . .",'z'});
    MorseAlpha.insert({ " ",' '});
}

//cette fonction sert à lire un fichier texte
std::string readTextFile(char* path) {
    
    std::fstream newfile;

    newfile.open(path, std::ios::in);

    std::string tp;
    std::string sortie;
    while(std::getline(newfile, tp)) {
        sortie.append(tp);

    }
    newfile.close();
    return sortie;
}

//cette fonction sert à écrire un fichier texte
void writeTextFile(char* path, std::string data) {
    std::fstream newfile;
    newfile.open(path,std::ios::out);  // open a file to perform write operation using file object
   
    newfile<<data; //inserting text
    newfile.close(); //close the file object
   
}

//cette fonction sert à lire les données d'un fichier wav
std::vector<int16_t> readData(const char* path, std::vector<float>& Lt){

    wav_hdr wavHeader;
    int headerSize = sizeof(wav_hdr), filelength = 0;

    FILE* wavFile = fopen(path, "r");
    size_t bytesRead = fread(&wavHeader, 1, headerSize, wavFile);

    std::vector<int16_t> data;

    if (bytesRead > 0) {
        uint16_t bytesPerSample = wavHeader.bitsPerSample / 8;      //Number     of bytes per sample
        uint64_t numSamples = wavHeader.ChunkSize / bytesPerSample; //How many samples are in the wav file?
        static const uint16_t BUFFER_SIZE = 2; //remplacer par 2 pour avoir le bon résultat, 4096 pour voir toute la sortie
        int16_t* buffer = new int16_t[BUFFER_SIZE];

        while ((bytesRead = fread(buffer, sizeof buffer[0], BUFFER_SIZE / (sizeof buffer[0]), wavFile)) > 0)
        {
            

            data.push_back((*buffer));


        }
        delete[] buffer;
        buffer = nullptr;
        

        fclose(wavFile);

        float t = 0.0f;

        float ut = 1 / float(wavHeader.SamplesPerSec);

        for (int i : data) {
            Lt.push_back(t);
            t += ut;
            
        }

        return data;
    }
    return std::vector<int16_t>(0);
}




//cette fonction sert à transformer une phrase en lettre en une suite de trait, de point et d'espace, 1 espace = 1 point muet
std::string alphaIntoMorse(std::string s) {
    /* Un tiret vaut 3 points, l'espacement entre 2 éléments d'une lettre vaut 1 point, 
    l'espacement entre 2 lettres vaut 3 points et l'espacement entre 2 mots faut 7 points*/

    std::string sortie = "";

    for (char t : s) {
        sortie.append(AlphaMorse[t]);
        
        sortie.append("   ");
    }
    sortie.append("       ");
    return sortie;
}

//cette fonction sert à transformer un message codé en morse en un message en lettre
std::string morseIntoAlpha(std::string s) {
    ///on suppose ici que s est une succession de traits, de points et de blancs
    

    std::vector<std::string>mots;
    std::vector<std::vector<std::string>> lLettres;
    std::string sortie = "";

    int nbBlancs = 0;
    char c;
    int n = 0;
    int a; int b;
    //on commence par séparer la chaîne de charactère en mots
    for (int i = 0; i < s.size();i++) {
        c = s[i];
        if (c == ' ') {
            nbBlancs += 1;
        }
        else {
            nbBlancs = 0;
        }
        if (nbBlancs == 7) {
            a= i - 7 - n + 1;
            b = s.size() - n;
            mots.push_back(s.substr(n,a).append("   "));
            n = i + 1;
            
            nbBlancs = 0;
        }
    }

    nbBlancs = 0;
    

    //on sépare chaque lettre de chaque mot
    for (int j = 0; j < mots.size(); j++) {
        lLettres.push_back(std::vector<std::string>());
        std::string mot = mots.at(j);
        n = 0;
        for (int i = 0; i < mot.size(); i++) {
            c = mot[i];
            if (c == ' ') {
                nbBlancs += 1;
            }
            else {
                nbBlancs = 0;
            }
            if (nbBlancs == 3) {
                lLettres[j].push_back(mot.substr(n, i - 3 - n + 1));
                n = i + 1;


            }
        }
    }
    
    
    //puis on reforme le mot
    for (int i = 0; i < lLettres.size(); i++) {
        for (int j = 0; j < lLettres[i].size(); j++) {
            sortie += MorseAlpha[lLettres[i][j]];
        }
        sortie.append(" ");
    }
    return sortie;
}



//cette fonction sert à écrire des données dans un fichier plus facilement
void writeToFile(std::ofstream& file, int value, int size) {
    file.write(reinterpret_cast<const char*> (&value), size);
}

int f = 200; //D'après le théorème de Niquyst-Shannon, il faut sampleRate>2*f
//la fréquence est très faible dûe à la fréquence d'échantillonage choisie très faible (pour faire des fichiers plus compact)
float amp=0.5;

//cette fonction sert à transformer un fichier en morse en fichier audio
void MorseIntoWav(std::string morse, const char* path) {
    
    SineOscillator osc(f, amp); //de base : f = 440


    std::ofstream audioFile;
    audioFile.open(path, std::ios::binary);

    //header chunk
    audioFile << "RIFF";
    audioFile << "----";
    audioFile << "WAVE";

    //format chunk
    audioFile << "fmt ";
    writeToFile(audioFile, 16, 4); //Size
    writeToFile(audioFile, 1, 2); //Compression code
    writeToFile(audioFile, 1, 2); //Number of Channels
    writeToFile(audioFile, sampleRate, 4); //SampleRate
    writeToFile(audioFile, sampleRate * bitDepth / 8, 4); //Byterate
    writeToFile(audioFile, bitDepth / 8, 2); //Block align
    writeToFile(audioFile, bitDepth, 2); //Bit depth

    //Data chunk
    audioFile << "data";
    audioFile << "----";

    int preAudioPosition = audioFile.tellp();

    auto maxAmplitude = pow(2, bitDepth - 1) - 1;

    //on va se baser sur une loop pour écrire tout l'audio, les blancs compris

    float duree = 0;
    int coeff = 1;
    for (char c : morse) {
        coeff = 1;
        if (c == ' ') {
            duree = t0;
            coeff = 0;
        }
        else if (c == '.') {
            duree = t0;
        }
        else {
            duree = 3 * t0;
        }

        for (int i = 0; i < sampleRate * duree; i++) {
            auto sample = osc.process();
            int intSample = sample * maxAmplitude * coeff;
            writeToFile(audioFile, intSample, 2);
        }
    }


    int postAudioPosition = audioFile.tellp();
    audioFile.seekp(preAudioPosition - 4);
    writeToFile(audioFile, postAudioPosition - preAudioPosition, 4);

    audioFile.seekp(4, std::ios::beg);
    writeToFile(audioFile, postAudioPosition - 8, 4);

    audioFile.close();

}

//cette fonction permet de lire un fichier wav et de le transformer en chaine de charactère en morse
std::string WavIntoMorse(std::vector<float> Lt, std::vector<int16_t> data) {
    std::string sortie="";
    float instant=0;
    std::vector<int>paquet;
    float  t; int d; int c; int mean;

    int threshold = (amp*pow(2, bitDepth - 1) - 1)/100; // 100 est une valeur empirique

    for(int i=0;i<data.size();i++) {
        t=Lt.at(i);
        d=data.at(i);
        //std::cout << "d = " << d << std::endl;
        if(t<instant+t0) {
            paquet.push_back(d);
        } else {
            c = std::count(paquet.begin(),paquet.end(),0);
            //std::cout << " c= " << c << " paquet.size = " << paquet.size() << std::endl;
            if (c>=float(paquet.size())/1.5) {
                //c'est un blanc
                sortie.append(" ");
            } else {
                //c'est du bruit
                sortie.append(".");
            }
            

            paquet.clear();
            instant+=t0;
        }

    }
    sortie.append(" ");
    
    int pos=sortie.find("...");
    while(pos != sortie.npos) {
        sortie.replace(pos,3,"-");
        pos=sortie.find("...");
    }
    
    return sortie;

}

//cette fonction gère toute la partie de la transcription alphabet vers morse
void Alpha_Morse() {
    std::string mode_Entree;
    std::cout << "voulez-vous entrer un fichier texte ? (Y/N)" << std::endl;
    std::getline(std::cin, mode_Entree);
    std::string input;
    char path1[100];
    std::string chaine;
    if (mode_Entree=="Y") {
        std::cout << "entrez le chemin du fichier" << std::endl;
        std::cin.get(path1,100);

        

        chaine=readTextFile(path1);
    } else if (mode_Entree=="N") {
        std::cout << "entrez un texte" << std::endl;
        std::getline(std::cin, chaine);
       
    }
    
    std::cout << "entrez un chemin de sortie" << std::endl;
    std::cin >> input;
    const char* path2=input.c_str();


    MorseIntoWav(alphaIntoMorse(chaine),path2);

    std::cout << "fichier audio créé au chemin : " << path2 << std::endl;

}

//cette fonction gère toute la partie de la transcription morse vers alphabet
void Morse_Alpha() {
    std::cout << "Entrez le chemin du fichier audio" << std::endl;
    char path1[100];
    std::cin.get(path1,100);

    std::vector <float> Lt;
    std::string sortie = morseIntoAlpha(WavIntoMorse(Lt,readData(path1,Lt)));

    std::cout << "le fichier lit : " << sortie << std::endl;
    std::cout << "Voulez-vous écrire ceci dans un fichier ? (Y/N)" << std::endl;
    std::string choice;
    std::cin >> choice;
    if (choice=="Y") {
        std::cout << "entrez un chemin" << std::endl;

        char path2[100];
        std::cin >> path2;
        
        writeTextFile(path2,sortie);
        std::cout << "fichier crée au chemin : " << path2 << std::endl;;
    }

}

//cette fonction sert à débuter la communication avec l'utilisateur
void communication() {
    std::cout << "voulez-vous encoder un message en alphabet vers morse (entrez am) ou de morse vers alphabet (entrez ma) ?" << std::endl;
    std::string input;
    std::getline(std::cin,input);

    while(!((input=="ma") || (input=="am"))) {
        std::cout << "entrée incorrecte" << std::endl;
        std::cout << "voulez-vous encoder un message en alphabet vers morse (entrez am) ou de morse vers alphabet (entrez ma) ?" << std::endl;

        std::cin >> input;        
    }

    if(input=="ma") {
        Morse_Alpha();
    } else {
        Alpha_Morse();
    }
    
}


int main(int argc, char** argv)
{

    fill_AlphaMorse();
    fill_MorseAlpha();

    /*std::string test = readTextFile("testWrite.txt");
    std::string resultat1 = alphaIntoMorse(test);
    std::cout << resultat1 << std::endl;
    std::string resultat2 = morseIntoAlpha(resultat1);
    std::cout << "résultat 2 |||" << resultat2 << "|||" << std::endl;

    const char* path = "test.wav";
    MorseIntoWav(resultat1, path);

    std::vector<float> Lt;
    std::vector<int16_t> data;
    data = readData(path, Lt);

    //for (int i = 0; i < data.size(); i++) {
    //    std::cout << "t = " << Lt.at(i) << " value : " << data.at(i) << std::endl;
    //}

    //std::cout << "t final : " << Lt.back() << std::endl;
    //std::cout << "nombre de valeur : " << data.size() << std::endl;

    std::ofstream dataFile("data.csv");
    for(int i=0;i<data.size();i++) {
        dataFile << Lt.at(i) << ";" << data.at(i) << std::endl;
        
    }

    std::string resultat3 = WavIntoMorse(Lt,data);

    std::string resultat4 = morseIntoAlpha(resultat3);


    std::cout << "résultat 4 |||" << resultat4 << "|||" << std::endl;

    writeTextFile("testWrite.txt",resultat4);
    */

   communication();

    return 0;
}

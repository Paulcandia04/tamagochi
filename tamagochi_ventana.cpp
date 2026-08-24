//  Compilar:
//     g++ -std=c++17 tamagochi_ventana.cpp -o tamagochi_ventana.exe -lsfml-graphics -lsfml-window -lsfml-system
//  Ejecutar:
//     tamagochi_ventana.exe


#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <optional>

using namespace std;

const string ARCHIVO_GUARDADO = "tamagochi_save.txt";

// sube energia y animo  mientras duerme.
const int RECUPERACION_POR_TICK = 5;

const double HORAS_POR_DIA = 4.0;

// Duracion deseada del dia
const double DURACION_DIA_SEGUNDOS = 15.0;
const double INCREMENTO_HORAS_POR_TICK = 0.1;
const double TICK_SEGUNDOS =
    INCREMENTO_HORAS_POR_TICK * DURACION_DIA_SEGUNDOS / (HORAS_POR_DIA / 2.0);

int clampInt(int valor, int minV, int maxV) {
    if (valor < minV) return minV;
    if (valor > maxV) return maxV;
    return valor;
}

enum class Etapa { Huevo, Bebe, Nino, Adolescente, Adulto };

string textoEtapa(Etapa e) {
    switch (e) {
        case Etapa::Huevo: return "Huevo";
        case Etapa::Bebe: return "Bebe";
        case Etapa::Nino: return "Nino";
        case Etapa::Adolescente: return "Adolescente";
        case Etapa::Adulto: return "Adulto";
    }
    return "?";
}

struct ResultadoAccion {
    bool ok;
    string mensaje;
};

class Mascota {
public:
    string nombre;
    int hambre, animo, energia, aseo, salud;
    double horasEdad;
    Etapa etapa;
    bool durmiendo;
    bool muerto;
    int cantidadCaca;
    long long ultimaActualizacion;

    //tamagochi nuevo
    Mascota(const string& nombreMascota = "Tamagochi")
        : nombre(nombreMascota),
          hambre(100), animo(100), energia(100), aseo(100), salud(100),
          horasEdad(0.0), etapa(Etapa::Huevo),
          durmiendo(false), muerto(false), cantidadCaca(0),
          ultimaActualizacion(static_cast<long long>(time(nullptr))) {}

    ResultadoAccion alimentar() {
        if (muerto) return { false, "Ya no esta con nosotros..." };
        if (durmiendo) return { false, "Esta dormido, no puede comer." };
        hambre = clampInt(hambre + 25, 0, 100);
        animo = clampInt(animo + 3, 0, 100);
        return { true, "Comiendo" };
    }

    ResultadoAccion jugar() {
        if (muerto) return { false, "Ya no esta con nosotros..." };
        if (durmiendo) return { false, "Esta dormido, no puede jugar." };
        if (energia < 10) return { false, "Esta muy cansado para jugar." };
        animo = clampInt(animo + 20, 0, 100);
        energia = clampInt(energia - 15, 0, 100);
        hambre = clampInt(hambre - 5, 0, 100);
        return { true, "Jugando" };
    }

    ResultadoAccion limpiar() {
        if (muerto) return { false, "Ya no esta con nosotros..." };
        aseo = 100;
        cantidadCaca = 0;
        return { true, "Limpiando" };
    }

    ResultadoAccion dormirDespertar() {
        if (muerto) return { false, "Ya no esta con nosotros..." };
        durmiendo = !durmiendo;
        return { true, durmiendo ? "Durmiendo" : "Despierto" };
    }

    void revivir(const string& nuevoNombre) {
        *this = Mascota(nuevoNombre);
    }


    vector<string> tick(bool esNoche) {
        vector<string> eventos;
        if (muerto) return eventos;

        horasEdad += INCREMENTO_HORAS_POR_TICK;

        if (durmiendo || esNoche) {
            energia = clampInt(energia + RECUPERACION_POR_TICK, 0, 100);
            animo = clampInt(animo + RECUPERACION_POR_TICK, 0, 100);
            hambre = clampInt(hambre - 1, 0, 100);
            aseo = clampInt(aseo - 1, 0, 100);
            salud = clampInt(salud + 1, 0, 100);
        } else {
            hambre = clampInt(hambre - 1, 0, 100);
            animo = clampInt(animo - 1, 0, 100);
            energia = clampInt(energia - 1, 0, 100);
            aseo = clampInt(aseo - 1, 0, 100);

            double azar = static_cast<double>(rand()) / RAND_MAX;
            if (azar < 0.02 && cantidadCaca < 4) {
                cantidadCaca += 1;
                aseo = clampInt(aseo - 8, 0, 100);
                eventos.push_back("Hizo del bano");
            }

            int descuido = (hambre < 20 ? 1 : 0) + (animo < 20 ? 1 : 0) +
                           (aseo < 20 ? 1 : 0) + (energia < 10 ? 1 : 0);

            if (descuido > 0) {
                salud = clampInt(salud - descuido * 2, 0, 100);
            } else if (hambre > 50 && aseo > 50 && animo > 50) {
                salud = clampInt(salud + 1, 0, 100);
            }

            if (salud <= 0) {
                muerto = true;
                eventos.push_back("Murio");
                return eventos;
            }
        }

        if (etapa != Etapa::Adulto) {
            if (horasEdad >= 2.0 && etapa == Etapa::Huevo) {
                etapa = Etapa::Bebe;
                eventos.push_back("Nacio!");
            } else if (horasEdad >= 8.0 && etapa == Etapa::Bebe) {
                etapa = Etapa::Nino;
                eventos.push_back("Crecio!");
            } else if (horasEdad >= 16.0 && etapa == Etapa::Nino) {
                etapa = Etapa::Adolescente;
                eventos.push_back("Crecio!");
            } else if (horasEdad >= 24.0 && etapa == Etapa::Adolescente) {
                etapa = Etapa::Adulto;
                eventos.push_back("Adulto!");
            }
        }
        return eventos;
    }

    int getDiasEdad() const { return static_cast<int>(horasEdad / HORAS_POR_DIA); }
};

namespace Guardado {
    bool guardar(const Mascota& mascota) {
        ofstream out(ARCHIVO_GUARDADO, ios::trunc);
        if (!out.is_open()) return false;
        out << "nombre=" << mascota.nombre << "\n";
        out << "hambre=" << mascota.hambre << "\n";
        out << "animo=" << mascota.animo << "\n";
        out << "energia=" << mascota.energia << "\n";
        out << "aseo=" << mascota.aseo << "\n";
        out << "salud=" << mascota.salud << "\n";
        out << "horasEdad=" << mascota.horasEdad << "\n";
        out << "etapa=" << textoEtapa(mascota.etapa) << "\n";
        out << "durmiendo=" << (mascota.durmiendo ? 1 : 0) << "\n";
        out << "muerto=" << (mascota.muerto ? 1 : 0) << "\n";
        out << "cantidadCaca=" << mascota.cantidadCaca << "\n";
        out << "ultimaActualizacion=" << mascota.ultimaActualizacion << "\n";
        return true;
    }

    bool cargar(Mascota& mascota) {
        ifstream in(ARCHIVO_GUARDADO);
        if (!in.is_open()) return false;
        string linea;
        while (getline(in, linea)) {
            auto pos = linea.find('=');
            if (pos == string::npos) continue;
            string key = linea.substr(0, pos);
            string value = linea.substr(pos + 1);
            istringstream iss(value);
            if (key == "nombre") mascota.nombre = value;
            else if (key == "hambre") iss >> mascota.hambre;
            else if (key == "animo") iss >> mascota.animo;
            else if (key == "energia") iss >> mascota.energia;
            else if (key == "aseo") iss >> mascota.aseo;
            else if (key == "salud") iss >> mascota.salud;
            else if (key == "horasEdad") iss >> mascota.horasEdad;
            else if (key == "etapa") {
                if (value == "Huevo") mascota.etapa = Etapa::Huevo;
                else if (value == "Bebe") mascota.etapa = Etapa::Bebe;
                else if (value == "Nino") mascota.etapa = Etapa::Nino;
                else if (value == "Adolescente") mascota.etapa = Etapa::Adolescente;
                else mascota.etapa = Etapa::Adulto;
            }
            else if (key == "durmiendo") { int v; iss >> v; mascota.durmiendo = v != 0; }
            else if (key == "muerto") { int v; iss >> v; mascota.muerto = v != 0; }
            else if (key == "cantidadCaca") iss >> mascota.cantidadCaca;
            else if (key == "ultimaActualizacion") iss >> mascota.ultimaActualizacion;
        }
        return true;
    }
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    sf::RenderWindow ventana(sf::VideoMode(sf::Vector2u(800, 650)), "Tamagochi", sf::Style::Close);
    ventana.setFramerateLimit(60);

    sf::Font fuente;
    if (!fuente.openFromFile("arial.ttf")) {
        if (!fuente.openFromFile("C:/Windows/Fonts/arial.ttf")) {
            cout << "No se encontro fuente." << endl;
        }
    }

    Mascota mascota;
    bool hayPartida = Guardado::cargar(mascota);
    string estadoTexto = hayPartida
        ? (mascota.durmiendo ? "Durmiendo" : "Despierto")
        : "Nacio!";

    bool corriendo = true;
    int contadorTicks = 0;

    sf::Clock relojTick;
    double acumuladorTick = 0.0;

    // Colores base cada barra 
    sf::Color colorHambre(255, 200, 100);
    sf::Color colorAnimo(255, 150, 200);
    sf::Color colorEnergia(100, 200, 255);
    sf::Color colorAseo(100, 255, 150);
    sf::Color colorSalud(255, 100, 100);
    sf::Color colorSalir(120, 60, 60);

    struct Boton {
        string texto;
        float x, y, w, h;
        sf::Color color;
    };

    // Botones al centro-derecha 
    float bx = 480.f, bw = 240.f, bh = 50.f, bgap = 15.f, byInicio = 100.f;
    vector<Boton> botones = {
        {"Comer",   bx, byInicio,                 bw, bh, colorHambre},
        {"Jugar",   bx, byInicio + (bh + bgap),     bw, bh, colorAnimo},
        {"Limpiar", bx, byInicio + (bh + bgap) * 2, bw, bh, colorAseo},
        {"Dormir",  bx, byInicio + (bh + bgap) * 3, bw, bh, colorEnergia},
        {"Nueva",   bx, byInicio + (bh + bgap) * 4, bw, bh, colorSalud},
        {"Salir",   bx, byInicio + (bh + bgap) * 5, bw, bh, colorSalir}
    };

    while (corriendo && ventana.isOpen()) {
        while (auto evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                corriendo = false;
            }
            if (const auto* mousePressed = evento->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    float x = static_cast<float>(mousePressed->position.x);
                    float y = static_cast<float>(mousePressed->position.y);

                    auto dentro = [&](const Boton& b) {
                        return x >= b.x && x <= b.x + b.w && y >= b.y && y <= b.y + b.h;
                    };

                    if (dentro(botones[0])) {
                        estadoTexto = mascota.alimentar().mensaje;
                    } else if (dentro(botones[1])) {
                        estadoTexto = mascota.jugar().mensaje;
                    } else if (dentro(botones[2])) {
                        estadoTexto = mascota.limpiar().mensaje;
                    } else if (dentro(botones[3])) {
                        estadoTexto = mascota.dormirDespertar().mensaje;
                    } else if (dentro(botones[4])) {
                        mascota.revivir("Tamagochi");
                        estadoTexto = "Nacio!";
                    } else if (dentro(botones[5])) {
                        corriendo = false;
                    }
                }
            }
        }

        // Avance de la simulacion (
        acumuladorTick += relojTick.restart().asSeconds();
        while (acumuladorTick >= TICK_SEGUNDOS) {
            acumuladorTick -= TICK_SEGUNDOS;
            contadorTicks++;

            auto eventos = mascota.tick(false);
            if (!eventos.empty()) {
                estadoTexto = eventos.back();
            }

            if (contadorTicks % 20 == 0) {
                mascota.ultimaActualizacion = static_cast<long long>(time(nullptr));
                Guardado::guardar(mascota);
            }
        }

        // Ciclo dia/noche 
        double faseDelDia = fmod(mascota.horasEdad, HORAS_POR_DIA);
        bool esDeDia = faseDelDia < (HORAS_POR_DIA / 2.0);
        sf::Color colorFondo = esDeDia ? sf::Color(255, 221, 89) : sf::Color(30, 60, 150);
        ventana.clear(colorFondo);

        sf::Color colorTexto(25, 25, 35);

        // Titulo 
        sf::Text titulo(fuente, "TAMAGOCHI", 30);
        titulo.setFillColor(colorTexto);
        titulo.setPosition(sf::Vector2f(280.f, 15.f));
        ventana.draw(titulo);

        // Estados arriba a la izquierda 
        sf::Text diaTxt(fuente, "Dia " + to_string(mascota.getDiasEdad()), 16);
        diaTxt.setFillColor(colorTexto);
        diaTxt.setPosition(sf::Vector2f(20.f, 20.f));
        ventana.draw(diaTxt);

        sf::Text estadoTxt(fuente, estadoTexto, 20);
        estadoTxt.setFillColor(colorTexto);
        estadoTxt.setPosition(sf::Vector2f(20.f, 50.f));
        ventana.draw(estadoTxt);

        if (mascota.cantidadCaca > 0) {
            sf::Text cacaTxt(fuente, "Caca x" + to_string(mascota.cantidadCaca), 14);
            cacaTxt.setFillColor(sf::Color(90, 55, 20));
            cacaTxt.setPosition(sf::Vector2f(20.f, 80.f));
            ventana.draw(cacaTxt);
        }

        //  n circulo con dos ojos
        sf::CircleShape cuerpo(80);
        cuerpo.setPosition(sf::Vector2f(260.f, 140.f));
        cuerpo.setFillColor(sf::Color(255, 224, 130));
        cuerpo.setOutlineColor(sf::Color(80, 80, 80));
        cuerpo.setOutlineThickness(2);
        ventana.draw(cuerpo);

        sf::CircleShape ojo1(10), ojo2(10);
        ojo1.setFillColor(sf::Color::Black);
        ojo2.setFillColor(sf::Color::Black);
        ojo1.setPosition(sf::Vector2f(290.f, 190.f));
        ojo2.setPosition(sf::Vector2f(390.f, 190.f));
        ventana.draw(ojo1);
        ventana.draw(ojo2);

        sf::Text info(fuente, mascota.nombre + " (" + textoEtapa(mascota.etapa) + ")", 16);
        info.setFillColor(colorTexto);
        info.setPosition(sf::Vector2f(255.f, 300.f));
        ventana.draw(info);

        // Barras bajo al centro 
        int valores[5] = { mascota.hambre, mascota.animo, mascota.energia, mascota.aseo, mascota.salud };
        const char* etiquetas[5] = { "Hambre", "Animo", "Energ", "Aseo", "Salud" };
        sf::Color coloresBarra[5] = { colorHambre, colorAnimo, colorEnergia, colorAseo, colorSalud };

        float barW = 40.f, barGap = 15.f, barAltura = 180.f, barTopY = 400.f;
        float totalAncho = 5 * barW + 4 * barGap;
        float barStartX = 300.f - totalAncho / 2.0f; // centrado bajo el personaje

        for (int i = 0; i < 5; ++i) {
            float bxPos = barStartX + i * (barW + barGap);

            sf::RectangleShape fondoBarra(sf::Vector2f(barW, barAltura));
            fondoBarra.setPosition(sf::Vector2f(bxPos, barTopY));
            fondoBarra.setFillColor(sf::Color(0, 0, 0, 60));
            fondoBarra.setOutlineColor(colorTexto);
            fondoBarra.setOutlineThickness(1);
            ventana.draw(fondoBarra);

            float alturaLlena = barAltura * (valores[i] / 100.0f);
            sf::RectangleShape relleno(sf::Vector2f(barW, alturaLlena));
            relleno.setPosition(sf::Vector2f(bxPos, barTopY + (barAltura - alturaLlena)));
            sf::Color colorRelleno = coloresBarra[i];
            if (valores[i] < 30) colorRelleno = sf::Color(220, 60, 60);
            else if (valores[i] < 60) colorRelleno = sf::Color(230, 180, 60);
            relleno.setFillColor(colorRelleno);
            ventana.draw(relleno);

            sf::Text porcentaje(fuente, to_string(valores[i]) + "%", 12);
            porcentaje.setFillColor(colorTexto);
            porcentaje.setPosition(sf::Vector2f(bxPos + 2.f, barTopY - 18.f));
            ventana.draw(porcentaje);

            sf::Text etiquetaTxt(fuente, etiquetas[i], 12);
            etiquetaTxt.setFillColor(colorTexto);
            etiquetaTxt.setPosition(sf::Vector2f(bxPos, barTopY + barAltura + 5.f));
            ventana.draw(etiquetaTxt);
        }

        // Botones al centro-derecha 
        for (auto& boton : botones) {
            sf::RectangleShape btn(sf::Vector2f(boton.w, boton.h));
            btn.setPosition(sf::Vector2f(boton.x, boton.y));
            btn.setFillColor(boton.color);
            btn.setOutlineColor(colorTexto);
            btn.setOutlineThickness(1);
            ventana.draw(btn);

            sf::Text texto(fuente, boton.texto, 16);
            texto.setFillColor(sf::Color(30, 30, 30));
            texto.setPosition(sf::Vector2f(boton.x + 15.f, boton.y + 13.f));
            ventana.draw(texto);
        }

        ventana.display();
    }

    mascota.ultimaActualizacion = static_cast<long long>(time(nullptr));
    Guardado::guardar(mascota);

    return 0;
}
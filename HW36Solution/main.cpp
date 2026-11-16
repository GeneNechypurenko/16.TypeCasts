#include <iostream>
#include <windows.h>
#include <string>
#include <typeinfo>

using namespace std;

/*
 ==========================================================================================================================================
                                  ДОМАШНЄ ЗАВДАННЯ 36 тема: перетворення типів static_cast і dynamic_cast.
 ==========================================================================================================================================
*/

class Piece
{
protected:
    string name;
    int value;                            // цінність фігури в пішаках

public:
    Piece(const string& name, int value) : name(name), value(value)
    {}

    virtual ~Piece() = default;

    string getName() const
    {
        return name;
    }

    int getValue() const
    {
        return value;
    }

    // Чисто віртуальний метод: кожна фігура ходить по-своєму, тому Piece — абстрактний клас
    virtual string getMoveRule() const = 0;

    virtual void print() const
    {
        cout << name << " (цінність " << value << "), ходить " << getMoveRule();
    }
};

// Пішак ходить лише вперед; дійшовши до 8-ї горизонталі, перетворюється на ферзя
class Pawn : public Piece
{
private:
    int row;                              // горизонталь: пішак стартує з 2-ї

public:
    explicit Pawn(int row) : Piece("Пішак", 1), row(row)
    {}

    void advance()
    {
        if (row < 8)
        {
            row++;
        }
        cout << "Пішак ходить на горизонталь " << row << endl;
    }

    /*
        4. Чому dynamic_cast працює лише тоді, коли в Piece є хоча б один віртуальний метод?
           І чому перетворення пішака — виправданий випадок для dynamic_cast?
    */
    bool canPromote() const
    {
        return row == 8;
    }

    string getMoveRule() const override
    {
        return "на 1 клітинку вперед";
    }

    double getProgressPercent() const     // скільки шляху від 2-ї до 8-ї горизонталі пройдено
    {
        return static_cast<double>(row - 2) * 100 / 6;   // перетворюємо операнд — ділення дробове
    }

    void print() const override
    {
        Piece::print();
        cout << ", горизонталь " << row << ", до перетворення пройдено " << getProgressPercent() << "%";
    }
};

class Knight : public Piece
{
public:
    Knight() : Piece("Кінь", 3)
    {}

    string getMoveRule() const override
    {
        return "буквою «Г»";
    }
};

class Bishop : public Piece
{
public:
    Bishop() : Piece("Слон", 3)
    {}

    string getMoveRule() const override
    {
        return "по діагоналі";
    }
};

class Rook : public Piece
{
public:
    Rook() : Piece("Тура", 5)
    {}

    string getMoveRule() const override
    {
        return "по вертикалі та горизонталі";
    }
};

class Queen : public Piece
{
public:
    Queen() : Piece("Ферзь", 9)
    {}

    string getMoveRule() const override
    {
        return "у будь-якому напрямку";
    }
};

// Середня цінність фігур у масиві
// 1. Чим static_cast<double>(sum) кращий за (double)sum? Чим небезпечне перетворення в стилі C?
// 2. Чому до виправлення getAverageValue повертала 2, а не 2.6? Чому static_cast<double>(sum / count) теж не допоможе?
double getAverageValue(Piece** pieces, int count)
{
    if (count == 0)
    {
        return 0;
    }
    int sum = 0;
    for (int i = 0; i < count; i++)
    {
        sum += pieces[i]->getValue();
    }
    return static_cast<double>(sum) / count;
}

// ==================== ЧАСТИНА 2 ====================

void pawnStep(Piece& piece)
{
    Pawn& pawn = dynamic_cast<Pawn&>(piece);   // не пішак — bad_cast вийде в main
    pawn.advance();
}

void printAll(Piece** pieces, int count)
{
    for (int i = 0; i < count; i++)
    {
        pieces[i]->print();                // віртуальний метод: кожна фігура виводиться по-своєму
        cout << endl;
    }
    cout << "Середня цінність: " << getAverageValue(pieces, count) << endl;
}

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    cout << "===== ЧАСТИНА 1 =====" << endl;
    Pawn pawn(6);
    pawn.print();                         // 66.6667%
    cout << endl;

    cout << endl << "===== ЧАСТИНА 2 =====" << endl;
    int count = 5;
    Piece** chess = new Piece * [count];
    chess[0] = new Pawn(7);
    chess[1] = new Knight();
    chess[2] = new Bishop();
    chess[3] = new Rook();
    chess[4] = new Pawn(4);
    printAll(chess, count);                // 2.6

    cout << endl;
    try
    {
        pawnStep(*chess[0]);               // пішак: 7 → 8
        pawnStep(*chess[1]);               // кінь — bad_cast
        cout << "Не виконається" << endl;
    }
    catch (const bad_cast&)
    {
        cout << "Ця фігура не пішак — так ходити не може" << endl;
    }

    cout << endl;
    for (int i = 0; i < count; i++)
    {
        /*
           3. Що повертає dynamic_cast для покажчика і що робить для посилання, якщо фігура не пішак?
              Що сталося б у кроці 3, якби замість dynamic_cast<Pawn*> написати static_cast<Pawn*>?
        */
        Pawn* pawnOnBoard = dynamic_cast<Pawn*>(chess[i]);   // не пішак — nullptr
        if (pawnOnBoard != nullptr && pawnOnBoard->canPromote())
        {
            delete chess[i];               // прибираємо пішака з дошки
            chess[i] = new Queen();        // на його місце — ферзь
            cout << "Пішак перетворився на ферзя!" << endl;
        }
    }

    cout << endl;
    printAll(chess, count);                // 4.2

    for (int i = 0; i < count; i++)
    {
        delete chess[i];
    }
    delete[] chess;

    return 0;
}

/*

  1. Чим static_cast<double>(sum) кращий за (double)sum? Чим небезпечне перетворення в стилі C?
  2. Чому до виправлення getAverageValue повертала 2, а не 2.6? Чому static_cast<double>(sum / count) теж не допоможе?
  3. Що повертає dynamic_cast для покажчика і що робить для посилання, якщо фігура не пішак?
     Що сталося б у кроці 3, якби замість dynamic_cast<Pawn*> написати static_cast<Pawn*>?
  4. Чому dynamic_cast працює лише тоді, коли в Piece є хоча б один віртуальний метод?
     І чому перетворення пішака — виправданий випадок для dynamic_cast?

  ВІДПОВІДІ НА ЧАСТИНУ «ПОЯСНИ» (еталон для розбору)

  1. 

  2. 

  3. 

  4. 

*/

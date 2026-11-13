#include <iostream>
#include <string>
#include <typeinfo>
#include <windows.h>

using namespace std;

/*
==========================================================================================================================================
                                                  Оператор dynamic_cast. RTTI і оператор typeid
==========================================================================================================================================

                                                              Оператор dynamic_cast
  У C++ при роботі з поліморфізмом діє просте правило: приведення вгору (upcasting) — від нащадка до базового класу — завжди дозволене
  і безпечне, адже кожен нащадок гарантовано має все, що є в базовому класі. А от приведення вниз (downcasting) — від базового класу до
  нащадка — за замовчуванням заборонене, оскільки покажчик на базовий клас не знає, який саме об'єкт насправді лежить у пам'яті.
  Проте іноді клас-нащадок має власний унікальний метод, якого немає в базовому класі. Щоб викликати його через покажчик на базовий клас,
  використовують оператор dynamic_cast.

  DYNAMIC_CAST — безпечне перетворення вниз (від базового класу до нащадка) з перевіркою реального типу ПІД ЧАС ВИКОНАННЯ.
  Працює лише з ПОЛІМОРФНИМИ класами — у базовому класі має бути хоча б один віртуальний метод (віртуальний деструктор теж рахується).

  Правила роботи з dynamic_cast
  - Для роботи dynamic_cast базовий клас обов'язково повинен мати хоча б одну віртуальну функцію (найчастіше це віртуальний деструктор).
    Інакше код просто не скомпілюється.
  - Покажчик: якщо реальний об'єкт не є цільовим типом або його нащадком, результат — nullptr. Його ОБОВ'ЯЗКОВО перевіряють.
  - Посилання: «нульових» посилань не буває, тому при невдачі кидається стандартний виняток bad_cast
    (нащадок exception, підключається з <typeinfo>).
  - Для перевірки типів у реальному часі використовується механізм RTTI (Run-Time Type Information). Це коштує часу: на кожен dynamic_cast
    програма шукає реальний тип об'єкта.

                                                                      RTTI
  RTTI (Run-Time Type Information) — інформація про реальний тип об'єкта, доступна під час виконання програми.
  Компілятор зберігає її разом із таблицею віртуальних функцій, тому вона є лише в поліморфних класів. На ній працює dynamic_cast.

                                                                 Оператор typeid
  TYPEID — використовується для отримання інформації про точний тип об'єкта або виразу під час виконання програми або на етапі компіляції.
  Як і dynamic_cast, він є частиною механізму RTTI і вимагає підключення заголовкового файлу <typeinfo>. Він вміє:
  1. Порівнювати типи двох об'єктів через оператор == (typeid == не враховує нащадків — перевіряє лише точний збіг типу):
     typeid(*person) == typeid(Student) - перевірка ТОЧНОГО збігу типу.
  2. Повертати технічне ім'я типу завдяки методу .name():
     typeid(*person).name() - назва реального типу; у Visual Studio: «class Student»

*/

class Person
{
protected:
    string name;

public:
    Person(const string& name) : name(name)
    {}

    virtual ~Person() = default;              // віртуальний метод робить клас поліморфним: dynamic_cast працює

    virtual void print() const
    {
        cout << name;
    }
};

class Student : public Person
{
private:
    int gradeSum;
    int gradeCount;

public:
    Student(const string& name) : Person(name), gradeSum(0), gradeCount(0)
    {}

    // Методи, яких немає в Person: через Person* їх не викликати
    void addGrade(int grade)
    {
        gradeSum += grade;
        gradeCount++;
    }

    double getAverage() const
    {
        if (gradeCount == 0)
        {
            return 0;
        }
        return static_cast<double>(gradeSum) / gradeCount;
    }

    void print() const override
    {
        cout << name << ", середній бал " << getAverage();
    }
};

// Староста — теж студент: нащадок нащадка
class GroupLeader : public Student
{
public:
    GroupLeader(const string& name) : Student(name)
    {}

    void print() const override
    {
        Student::print();
        cout << " (староста)";
    }
};

class Teacher : public Person
{
private:
    string subject;

public:
    Teacher(const string& name, const string& subject) : Person(name), subject(subject)
    {}

    string getSubject() const
    {
        return subject;
    }

    void print() const override
    {
        cout << name << ", викладає " << subject;
    }
};

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    int count = 4;
    Person** people = new Person * [count];
    people[0] = new Student("Олена Коваль");
    people[1] = new Teacher("Ігор Кравченко", "C++");
    people[2] = new GroupLeader("Марко Бондар");
    people[3] = new Student("Софія Ткач");

    cout << "1. dynamic_cast покажчика: nullptr, якщо тип не той" << endl;
    // Student* student = people[0];         // ПОМИЛКА: неявно вниз не можна
    for (int i = 0; i < count; i++)
    {
        Student* student = dynamic_cast<Student*>(people[i]);
        if (student != nullptr)               // Student або його нащадок — GroupLeader теж
        {
            student->addGrade(10);
            student->addGrade(11);
        }
    }
    for (int i = 0; i < count; i++)
    {
        cout << typeid(*people[i]).name() << ": ";   // реальний тип об'єкта
        people[i]->print();
        cout << endl;
    }

    cout << endl << "2. typeid == проти dynamic_cast" << endl;
    int exactStudents = 0;
    int allStudents = 0;
    for (int i = 0; i < count; i++)
    {
        if (typeid(*people[i]) == typeid(Student))   // ТОЧНИЙ збіг: староста не рахується
        {
            exactStudents++;
        }
        if (dynamic_cast<Student*>(people[i]) != nullptr)   // «Є студентом»: староста рахується
        {
            allStudents++;
        }
    }
    cout << "typeid == Student: " << exactStudents << ", dynamic_cast<Student*>: " << allStudents << endl;   // 2 і 3

    cout << endl << "3. dynamic_cast посилання: виняток bad_cast" << endl;
    try
    {
        Teacher& teacher = dynamic_cast<Teacher&>(*people[1]);   // people[1] — викладач: успіх
        cout << "Предмет: " << teacher.getSubject() << endl;
        Teacher& wrong = dynamic_cast<Teacher&>(*people[0]);     // people[0] — студентка: bad_cast
        cout << "Не виконається: " << wrong.getSubject() << endl;
    }
    catch (const bad_cast& e)                 // стандартний виняток, нащадок exception
    {
        cout << "bad_cast: " << e.what() << endl;
    }

    cout << endl << "4. static_cast вниз: без перевірки" << endl;
    Student* olena = static_cast<Student*>(people[0]);   // реальний тип ТОЧНО відомий — перевірка не потрібна
    cout << "Середній бал Олени: " << olena->getAverage() << endl;
    // Student* wrong = static_cast<Student*>(people[1]);   // компілюється! Але це Teacher — невизначена поведінка

    for (int i = 0; i < count; i++)
    {
        delete people[i];
    }
    delete[] people;

    return 0;
}

/*
==========================================================================================================================================
                                                  ІНСАЙТИ / ЛАЙФХАКИ / BEST PRACTICES
==========================================================================================================================================

  1. Багато dynamic_cast у коді — ознака поганого дизайну. Замість «якщо студент — зроби так, якщо викладач — інакше»
     має бути віртуальний метод. dynamic_cast потрібен у рідкісних випадках, коли треба викликати метод, який є лише
     в нащадка (як addGrade).
     Це пов'язано з принципом підстановки Лісков (LSP, буква «L» у SOLID — п'яти принципах проєктування ООП, про які
     часто питають на співбесідах): будь-який нащадок має правильно працювати там, де очікують базовий клас.
     Якщо функції з параметром Person& доводиться з'ясовувати реальний тип через dynamic_cast чи typeid, щоб
     поводитися по-різному, — це симптом порушення LSP: нащадки не є повноцінною заміною базового класу, і кожен
     новий клас змусить шукати й доповнювати всі такі перевірки. Перетворення ВГОРУ (до базового класу) — нормальне
     і навіть неявне; погана ознака — саме перетворення ВНИЗ усередині функцій, що працюють з базовим класом.

  2. Без жодного віртуального методу dynamic_cast вниз не скомпілюється. На практиці це рідко проблема:
     базовому класу ієрархії все одно потрібен віртуальний деструктор.

  3. Бонус: у реальному коді часто оголошують змінну прямо в умові if — вона існує лише всередині цього if:
         if (Student* student = dynamic_cast<Student*>(people[i])) { student->addGrade(10); }

  4. typeid у прикладному коді трапляється рідко: здебільшого для налагодження і журналів. Не порівнюйте рядки name():
     у різних компіляторах формат різний (Visual Studio — «class Student», g++ — «7Student»). Порівнюйте самі typeid.

  5. У C# ті самі ідеї: person as Student — як dynamic_cast покажчика (null при невдачі), (Student)person — як dynamic_cast
     посилання (виняток при невдачі), person is Student — перевірка типу.

  6. static_cast теж уміє перетворювати вниз, але БЕЗ перевірки це може призвести до невизначеної поведінки. Тому його використовують лише
     тоді, коли реальний тип ТОЧНО відомий.

==========================================================================================================================================
                                                              ПРАКТИКА
==========================================================================================================================================

  Завдання 2 (продовжуємо загін героїв).
  У main:
    - створіть динамічний масив Hero** на 3 героїв: маг «Мерлін» (45 з 120, мана 50), воїн «Конан» (100 з 100),
      маг «Моргана» (80 з 100, мана 20);
    - у циклі через dynamic_cast покажчика: якщо герой — маг, викликайте castFireball(),
      інакше виводьте «<name> не вміє чаклувати»;
    - у try перетворіть *party[1] на Mage& через dynamic_cast і викличте castFireball();
      catch (const bad_cast&) виводить «Цей герой — не маг»;
    - звільніть пам'ять.
*/

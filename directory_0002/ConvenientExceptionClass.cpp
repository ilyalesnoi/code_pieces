/*
    Удобная иерархия кастомных классов исключений.
    Можно пользоваться так: throw ParserStructureException("text") << " detaildet text" << someNativeTypeValue;

    На этом примере мне нравится как удобно работает механизм шаблонов вкупе с повышающим преобразованием.

    Из интересного, если бы объект ислкючения создавался в локальной переменной а потом использовался бы вызов
    throw createdException; то не потребовалось бы шаблона оператора << в классах ParserSemanticException, ParserStructureException

    Компилятор при встрече объекта createdException (type ParserSemanticException) - при виде оператора <<
    вызывал бы ParserException::operator<< - оператор базового класса.

    В слуае констрирования исключения непосредственно в throw строке - тоже работает такое приведение к базе и вызов нужного метода
    Но вероятно это происходи или позднее (за счет приведения) или типа того - внутренняя каша не ясна. В итоге (если сделать дебагвывод)
    операторы сработали но апа при таком исключении упадет и не выдаст текст.

    Если же обеспечить цепочку схожих шаблонов оператора << для производных классов - то все будет работать как и задумано, и можно очень удобно
    детализировано описывать исключительные ситуации.
*/

#pragma once
#include <string>

namespace FileParser {

/**
 * @class Exception
 * Базовый класс для рождаемых исключений парсерами.
 */
class Exception {
private:
    std::wstring m_text;
public:
    Exception() = default;

    explicit Exception(const std::wstring &str) : m_text(str) {
    }

    explicit Exception(const std::string &str) : m_text(Utils::Misc::toStdWString(str)) {
    }

    Exception(const wchar_t *text) : m_text(text) {
    }

    explicit Exception(const char *text) {
        m_text.append(Utils::Misc::toStdWString(text));
    }

    virtual ~Exception() {

    }

    Exception &operator<<(size_t digit) {
        m_text.append(std::to_wstring(digit));
        return *this;
    }

    Exception &operator<<(int digit) {
        m_text.append(std::to_wstring(digit));
        return *this;
    }

    Exception &operator<<(const char *text) {
        m_text.append(Utils::Misc::toStdWString(text));
        return *this;
    }

    Exception &operator<<(char symbol) {
        std::wstring _s;
        _s += static_cast<wchar_t>(symbol);
        m_text.append(_s);
        return *this;
    }

    Exception &operator<<(wchar_t symbol) {
        std::wstring _s;
        _s += symbol;
        m_text.append(_s);
        return *this;
    }

    Exception &operator<<(const wchar_t *text) {
        m_text.append(text);
        return *this;
    }

    Exception &operator<<(const std::wstring &text) {
        m_text.append(text);
        return *this;
    }

    Exception &operator<<(const std::string &text) {
        m_text.append(Utils::Misc::toStdWString(text));
        return *this;
    }

    const std::wstring &what() const {
        return m_text;
    }
};

/**
 * @class ParserException
 * Исключение, которое бросается в случае внутренней ошибки парсера.
 */
class ParserException : public Exception {
public:
    ParserException() = default;

    template <typename _CHAR_>
    explicit ParserException(const _CHAR_ &ch): Exception(ch) {

    }

    template<typename T>
    ParserException &operator<<(const T &t) {
        static_cast<Exception&>(*this) << t;
        return *this;
    }

    virtual ~ParserException() {

    }
};

/**
 * @class ParserStructureException
 * Исключение, которое бросается в случае некорректной структуры файла.
 */
class ParserStructureException : public ParserException {
public:
    ParserStructureException() = default;
    template <typename _CHAR_> explicit ParserStructureException(const _CHAR_ &ch): ParserException(ch) {
    }

    template<typename T>
    ParserStructureException &operator<<(const T &t) {
        static_cast<ParserException&>(*this) << t;
        return *this;
    }

    virtual ~ParserStructureException() {}
};

/**
 * @class ParserSemanticException
 * Исключение, которое бросается в случае некорректного семантического состава.
 */
class ParserSemanticException : public ParserException {
public:
    ParserSemanticException() = default;
    template <typename _CHAR_> explicit ParserSemanticException(const _CHAR_ &ch): ParserException(ch) {

    }

    template<typename T>
    ParserSemanticException &operator<<(const T &t) {
        static_cast<ParserException&>(*this) << t;
        return *this;
    }

    virtual ~ParserSemanticException() {}
};


} // namespace FileParser

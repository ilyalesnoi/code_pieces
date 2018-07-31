/*
    Был некоторый класс в котором описаны свойства символа, для операций сравнения символа с другим символам
    по более широким критериям идентичности, чем просто байтовое соотвествие. Для этого в классе существовало
    3 классических массива, которые хранили признаки сравнения (два из них просты - это wchart_t, другой хранит указатель)
    на некотрый тип, который создается для определенного множества символов, например символов табуляции.
    
    Массивы, хранящие признаки, были огроными (чтобы покрывать всё множество utf-8), но фактически были нужны из этих массивов
    намного меньше элементов чем в них было. Так же конструирование элементов происходило во всем массиве.


    Класс CharacterTypeSetData, решает проблему тем что будет выделено памяти не больше чем индекс самого последнего символа,
    который нашелся где-то в тексте и был добавлен клиентом класса.

    Доступ к элеменам обеспечивается такой почти же быстрый (кроме времени 2х арифметических операций, и +1 перехода по адресу)
    как для классических массиов.
    
    Правда в последстии выбор пал на std::unordered_map и от этого класса отказались.
    unordered_map имеет константное время доступа, и если не итерироваться по итераторам, а обращаться по ключам - то, вероятно,
    не меньшее быстродействие чем данный контейнер.
    
    Каждый элемент unordered_map тратит памяти значительно больше (см его реализации в инете), чем каждый элемент данного класса,
    но при этом сам unordered_map будет хранить только нужное кол-во элементов, а данный класс все же будет те же разряженным массивом
    что и в первичном решении (только "урезанным сверху") и согласно статистике - пустые расстояния получаюся весьма длинными на
    тех наборах текста, с которым работала программа. Поэтому не смотря на то что для хранения каждого элемента памяти тратится больше
    памяти, в итоге unordered_map потратит меньше памяти.
*/

#pragma once

class CharacterTypeSetData{
    static const size_t _default_container_size_;

    struct Coordinates {
        size_t i, j;
    };

    struct CharacterTypeTraits {
        const CharacterType *m_characterTypePtr;
        wchar_t m_compareValue;
        wchar_t m_caseInsensetiveCompareValue;
        CharacterTypeTraits( ) :
            m_characterTypePtr(nullptr)
          , m_compareValue(0)
          , m_caseInsensetiveCompareValue(0) { }
    };

private:
    const CharacterType *m_defaultCharacterType;
    CharacterTypeTraits **m_data;
    size_t i_size;
    size_t j_size;

    size_t get_i(size_t index) const { return index / j_size; }
    size_t get_j(size_t index) const { return index % j_size; }

    void _reallocate(size_t new_size_i) {
        CharacterTypeTraits **m_data_new = new CharacterTypeTraits* [new_size_i];
        for (size_t i = 0; i < new_size_i; i++) {
            if (m_data && i < i_size) {
                m_data_new[i] = m_data[i];
            } else {
                m_data_new[i] = new CharacterTypeTraits[j_size];
            }
        }
        if (m_data)
            delete [] m_data;
        i_size = new_size_i;
        m_data = m_data_new;
    }

    void createDefaultCharacterTypeInPosition(size_t i, size_t j, wchar_t iCharacter)
    {
        if (i >= i_size) {
            _reallocate(i+1);
        }
        m_data[i][j].m_characterTypePtr = m_defaultCharacterType;
        m_data[i][j].m_compareValue = iCharacter;
        m_data[i][j].m_caseInsensetiveCompareValue = iCharacter;
    }

    void setDafaultCharacterTypeInPosition(size_t i, size_t j, wchar_t iCharacter)
    {
        m_data[i][j].m_characterTypePtr = m_defaultCharacterType;
        m_data[i][j].m_compareValue = iCharacter;
        m_data[i][j].m_caseInsensetiveCompareValue = iCharacter;
    }

    Coordinates touchElement(wchar_t iCharacter)
    {
        size_t i = get_i(static_cast<size_t>(iCharacter));
        size_t j = get_j(static_cast<size_t>(iCharacter));
        if (i >= i_size) {
            createDefaultCharacterTypeInPosition(i,j,iCharacter);
        }
        if (m_data[i][j].m_characterTypePtr == nullptr) {
            setDafaultCharacterTypeInPosition(i,j,iCharacter);
        }
        return {i, j};
    }

    void clear()
    {
        for (size_t i = 0; i < i_size; i++)
            delete [] m_data[i];
        delete [] m_data;
    }

public:
    explicit CharacterTypeSetData(
            const CharacterType *defaultCharacterType = nullptr,
            size_t initialContainerSize = _default_container_size_)
        : m_defaultCharacterType(defaultCharacterType)
        , m_data(nullptr)
        , i_size(0)
        , j_size(initialContainerSize)
    {
        _reallocate(i_size);
    }

    ~CharacterTypeSetData( )
    {
        clear();
    }

    const CharacterTypeSetData& operator=(const CharacterTypeSetData &rhs)
    {
        if (this != &rhs) {
            if (m_data) {
                clear();
                m_data = nullptr;
                i_size = rhs.i_size;
                j_size = rhs.j_size;
                m_defaultCharacterType = rhs.m_defaultCharacterType;
                if (rhs.m_data) {
                    m_data = new CharacterTypeTraits*[i_size];
                    for (size_t i = 0; i < i_size; i++) {
                        m_data[i] = new CharacterTypeTraits[j_size];
                        memcpy(m_data[i], rhs.m_data[i], sizeof(CharacterTypeTraits)*j_size);
                    }
                }
            }
        }
        return *this;
    }

    const CharacterType* getCharacterType(wchar_t iCharacter)
    {
        Coordinates coord = touchElement(iCharacter);
        return m_data[coord.i][coord.j].m_characterTypePtr;
    }

    wchar_t getCompareCharacter(wchar_t iCharacter)
    {
        Coordinates coord = touchElement(iCharacter);
        return m_data[coord.i][coord.j].m_compareValue;
    }

    wchar_t getCompareCharacterInsensetive(wchar_t iCharacter)
    {
        Coordinates coord = touchElement(iCharacter);
        return m_data[coord.i][coord.j].m_caseInsensetiveCompareValue;
    }

    void setCharacterType(wchar_t index, const CharacterType *ptr)
    {
        Coordinates coord = touchElement(index);
        m_data[coord.i][coord.j].m_characterTypePtr = ptr;
    }

    void setCompareCharacter(wchar_t index, wchar_t iCharacter)
    {
        Coordinates coord = touchElement(index);
        m_data[coord.i][coord.j].m_compareValue = iCharacter;
    }

    void setCompareCharacterInsensetive(wchar_t index, wchar_t iCharacter)
    {
        Coordinates coord = touchElement(index);
        m_data[coord.i][coord.j].m_caseInsensetiveCompareValue = iCharacter;
    }
};


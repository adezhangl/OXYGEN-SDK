// Copyright DEWETRON GmbH 2014

#pragma once

#include "odkapi_types.h"
#include "odkapi_version_xml.h"
#include "odkuni_defines.h"

#include <boost/lexical_cast.hpp>
#include <boost/rational.hpp>
#include <boost/shared_ptr.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace odk
{
    class PropertyList;
    struct Scalar
    {
        Scalar() noexcept : m_val(0) {}
        Scalar(double val, std::string unit);

        ODK_NODISCARD bool operator==(Scalar const& other) const;

        double m_val;
        std::string m_unit;
    };

    struct Rational
    {
        using value_type = boost::rational<int64_t>;
        Rational();

        Rational(const value_type& val, const std::string& unit);

        bool operator==(Rational const& other) const;

        value_type m_val;
        std::string m_unit;
    };

    struct DecoratedNumber
    {
        DecoratedNumber();

        DecoratedNumber(const double& val, const std::string& prefix, const std::string& suffix);

        bool operator==(DecoratedNumber const& other) const;

        double m_val;
        std::string m_prefix;
        std::string m_suffix;
    };

    struct Range
    {
        Range();
        Range(const double& min, const double& max, const std::string& min_unit, const std::string& max_unit);
        Range(const double& min, const double& max, const std::string& unit);

        bool operator==(Range const& other) const;
        bool operator!=(Range const& other) const;

        double m_min;
        double m_max;
        std::string m_min_unit;
        std::string m_max_unit;
    };

    template<class VT>
    struct ValueList
    {
        typedef VT ValueType;
        typedef std::vector<ValueType> ListType;

        ValueList()
            : m_values()
        {}

        explicit ValueList(const ListType& values)
            : m_values(values)
        {
        }

        explicit ValueList(ListType&& values)
            : m_values(std::move(values))
        {
        }

        explicit ValueList(std::initializer_list<ValueType> list)
            : m_values(list)
        {
        }

        template<class Iter>
        explicit ValueList(Iter begin, Iter end)
            : m_values(begin, end)
        {
        }

        bool operator==(ValueList<ValueType> const& rhs) const
        {
            if (m_values.size() != rhs.m_values.size())
            {
                return false;
            }

            auto it = m_values.begin();
            auto rhs_it = rhs.m_values.begin();
            for (; it != m_values.end(); ++it, ++rhs_it)
            {
                if (*it != *rhs_it)
                {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(ValueList<ValueType> const& rhs) const
        {
            return !(*this == rhs);
        }

        ListType m_values;
    };
    typedef ValueList<double> DoubleList;
    typedef ValueList<std::string> StringList;
    typedef std::pair<double, double> Point;
    typedef ValueList<Point> PointList;
    typedef ValueList<ChannelID> ChannelIDList;

    /**
     * @brief The Property class holds a generic name/value pair that can be
     * converted from/to XML
     *
     * In addition the type of the value is stored as well
     */
    class Property
    {
    public:
        enum Type {
            UNKNOWN = 0,
            STRING = 1,
            INTEGER = 2,
            UNSIGNED_INTEGER = 3,
            UNSIGNED_INTEGER64 = 4,
            FLOATING_POINT_NUMBER = 5,
            BOOLEAN = 6,
            COLOR = 7,
            DATE = 8,
            DATETIME = 9,
            RANGE = 10,
            ENUM = 11,
            PROPERTY_LIST = 12,
            SCALAR = 13,
            STRING_LIST = 14,
            POINT_LIST = 15,
            DECORATED_NUMBER = 16,
            CHANNEL_ID = 17,
            FLOATING_POINT_NUMBER_LIST = 18,
            RATIONAL = 19,
            GEO_COORDINATE = 20,
            POINT = 21,
            CHANNEL_ID_LIST = 22,
            INTEGER64 = 23,
        };

        enum StringFormat {
            STRING_UNKNOWN,
            STRING_PLAIN,
            STRING_XML,
            STRING_MULTILINE,
            STRING_RST,
        };

        static Type getPropertyTypeFromValue(const Scalar&) { return SCALAR; }
        static Type getPropertyTypeFromValue(const DecoratedNumber&) { return DECORATED_NUMBER; }
        static Type getPropertyTypeFromValue(const Range&) { return RANGE; }
        static Type getPropertyTypeFromValue(const DoubleList&) { return FLOATING_POINT_NUMBER_LIST; }
        static Type getPropertyTypeFromValue(const StringList&) { return STRING_LIST; }
        static Type getPropertyTypeFromValue(const Point&) { return POINT; }
        static Type getPropertyTypeFromValue(const PointList&) { return POINT_LIST; }
        static Type getPropertyTypeFromValue(const Rational&) { return RATIONAL; }
        static Type getPropertyTypeFromValue(const ChannelIDList&) { return CHANNEL_ID_LIST; }
        static Type getPropertyTypeFromValue(const PropertyList&) { return PROPERTY_LIST; }
        static Type getPropertyTypeFromValue(const std::string&) { return STRING; }
        static Type getPropertyTypeFromValue(const char*) { return STRING; }
        static Type getPropertyTypeFromValue(unsigned int) { return UNSIGNED_INTEGER; }
        static Type getPropertyTypeFromValue(bool) { return BOOLEAN; }
        static Type getPropertyTypeFromValue(int) { return INTEGER; }
        static Type getPropertyTypeFromValue(std::int64_t) { return INTEGER64; }
        static Type getPropertyTypeFromValue(std::uint64_t) { return UNSIGNED_INTEGER64; }
        static Type getPropertyTypeFromValue(double) { return FLOATING_POINT_NUMBER; }

        Property() noexcept;
        Property(std::string name) noexcept;

        Property(std::string name, std::string string_value) noexcept;
        Property(std::string name, const char* string_value);
        Property(std::string name, bool value);
        Property(std::string name, int value);
        Property(std::string name, unsigned int value);
        Property(std::string name, std::string value, std::string enum_type) noexcept;
        Property(const std::string& name, Type type, const std::string& value);

        template <class T>
        Property(std::string name, T value)
            : m_name(std::move(name))
            , m_type(getPropertyTypeFromValue(value))
            , m_value(std::make_shared<T>(std::move(value)))
        {
        }
        virtual ~Property() = default;

        ODK_NODISCARD std::string getNodeName() const;

        ODK_NODISCARD bool operator==(Property const& other) const;
        ODK_NODISCARD bool sameValue(Property const& other) const;

        template <typename T, typename=void>
        struct IsLexCastable : std::false_type {};

        template <typename T>
        struct IsLexCastable<T,
                             decltype(void(std::declval<std::ostream&>() << std::declval<T>()))>
            : std::true_type {};

        template<class PROPERTY_TYPE>
        typename std::enable_if<IsLexCastable<PROPERTY_TYPE>::value, PROPERTY_TYPE>::type cast() const
        {
            try
            {

                if(m_type == BOOLEAN)
                {
                    return boost::lexical_cast<PROPERTY_TYPE>(getBoolValue());
                }
                if(m_value)
                {
                    return *std::static_pointer_cast<PROPERTY_TYPE>(m_value);
                }
                else
                {
                    return boost::lexical_cast<PROPERTY_TYPE>(m_string_value);
                }
            }
            catch(...)
            {
                throw std::runtime_error("");
            }
        }

        template<class PROPERTY_TYPE>
        typename std::enable_if<!IsLexCastable<PROPERTY_TYPE>::value, PROPERTY_TYPE>::type cast() const
        {
            if(m_value)
            {
                return *std::static_pointer_cast<PROPERTY_TYPE>(m_value);
            }
            throw std::runtime_error("");
        }

        template<class PROPERTY_TYPE>
        PROPERTY_TYPE getValueStrict() const
        {
            if (m_type != getPropertyTypeFromValue(PROPERTY_TYPE()))
            {
                throw std::runtime_error("");
            }
            return cast<PROPERTY_TYPE>();
        }

        template<class PROPERTY_TYPE>
        PROPERTY_TYPE getValue() const
        {
            if (m_value && m_type != getPropertyTypeFromValue(PROPERTY_TYPE()))
            {
                throw std::runtime_error("");
            }
            return cast<PROPERTY_TYPE>();
        }


        /**
         * Returns true, if the data in the property is valid
         */
        ODK_NODISCARD bool isValid() const;

        /**
         * Sets the name of this property as string
         */
        void setName(std::string name) noexcept;
        /**
         * Returns the name of this property as string
         */
        ODK_NODISCARD const std::string& getName() const;

        /**
         * Returns the type of this property
         */
        ODK_NODISCARD Type getType() const;

        ODK_NODISCARD StringFormat getStringFormat() const;

        /**
         * Sets the value of this property as string
         */
        void setValue(const std::string& value);
        void setValue(const std::string& value, StringFormat format);

        /**
         * Returns the value of this property as string
         * This method always returns the property as string, regardless of it's type
         */
        ODK_NODISCARD const std::string& getStringValue() const;

        /**
         * Sets the value of this property as const char
         */
        void setValue(const char* value);

        /**
         * Sets the value of this property as int
         */
        void setValue(int value);
        /**
         * Returns the value of this property as int
         * If this property is not of type INTEGER, an exception is thrown
         */
        ODK_NODISCARD int getIntValue() const;

        /**
         * Sets the value of this property as unsigned int
         */
        void setValue(unsigned int value);
        /**
         * Returns the value of this property as int
         * If this property is not of type UNSIGNED_INTEGER, an exception is thrown
         */
        ODK_NODISCARD unsigned int getUnsignedIntValue() const;

        /**
         * Sets the value of this property as unsigned int 64 bit
         */
        void setValue(std::uint64_t value);
        /**
         * Returns the value of this property as uint64
         * If this property is not of type UNSIGNED_INTEGER[64], an exception is thrown
         */
        ODK_NODISCARD std::uint64_t getUnsignedInt64Value() const;

        /**
         * Sets the value of this property as signed int 64 bit
         */
        void setValue(std::int64_t value);
        /**
         * Returns the value of this property as sint64
         * If this property is not of type INTEGER[64], an exception is thrown
         */
        ODK_NODISCARD std::int64_t getInt64Value() const;

        /**
         * Sets the value of this property as double
         */
        void setValue(double value);

        /**
         * Returns the value of this property as double
         * If this property is not of type FLOATING_POINT_NUMBER, an exception is thrown
         */
        ODK_NODISCARD double getDoubleValue() const;

        /**
         * Sets the value of this property as bool
         */
        void setValue(bool value);
        /**
         * Returns the value of this property as bool
         * If this property is not of type BOOLEAN, an exception is thrown
         */
        ODK_NODISCARD bool getBoolValue() const;

        /**
         * Sets the value of this property as scalar
         */
        void setValue(const Scalar& value);

        /**
         * Returns the value of this property as Scalar
         * If this property is not of type SCALAR, an exception is thrown
         */
        ODK_NODISCARD const Scalar& getScalarValue() const;

        void setValue(const DecoratedNumber& value);

        /**
         * Returns the value of this property as DecoratedNumber
         * If this property is not of type DECORATED_NUMBER, an exception is thrown
         */
        ODK_NODISCARD const DecoratedNumber& getDecoratedNumberValue() const;

        /**
         * Sets the value of this property as range
         */
        void setValue(const Range& value);

        /**
         * Returns the value of this property as Range
         * If this property is not of type RANGE, an exception is thrown
         */
        ODK_NODISCARD const Range& getRangeValue() const;

        /**
         * Sets the value of this property as enum
         */
        void setEnumValue(std::string value, std::string enum_type);

        /**
         * Returns the value of this property as string
         * If this property is not of type ENUM, an exception is thrown
         */
        ODK_NODISCARD const std::string& getEnumValue() const;
        /**
         * Returns the enum type of this property as string
         * If this property is not of type ENUM, an exception is thrown
         */
        ODK_NODISCARD const std::string& getEnumType()const;

        /**
         * Sets the value of this property as PropertyList
         */
        void setValue(const PropertyList& value);

        /**
        * Sets the value of this property as DoubleList
        */
        void setValue(const DoubleList& value);

        /**
        * Returns the value of this property as DoubleList
        * If this property is not of type FLOATING_POINT_NUMBER_LIST, an exception is thrown
        */
        ODK_NODISCARD DoubleList getDoubleListValue() const;

        /**
         * Sets the value of this property as StringList
         */
        void setValue(const StringList& value);

        /**
         * Returns the value of this property as StringList
         * If this property is not of type STRING_LIST, an exception is thrown
         */
        ODK_NODISCARD StringList getStringListValue() const;

        /**
         * Sets the value of this property as Point
         */
        void setValue(const Point& value);

        ODK_NODISCARD const Point& getPointValue() const;

        /**
         * Sets the value of this property as PointList
         */
        void setValue(const PointList& value);

        /**
         * Sets the value of this property as boost::rational
         */
        void setValue(const Rational& value);

        ODK_NODISCARD const Rational& getRationalValue() const;

        void setChannelIDValue(const ChannelID& value);

        ODK_NODISCARD ChannelID getChannelIDValue() const;

        void setChannelIDListValue(const ChannelIDList& value);

        ODK_NODISCARD ChannelIDList getChannelIDListValue() const;

        void setDateValue(const std::string& date);
        void setDateTimeValue(const std::string& date_time);
        void setColorValue(const std::string& color);
        void setGeoCoordinateValue(const std::string& coord);
        /**
         * Returns the value of this property as string
         * If this property is not of type POINT_LIST, an exception is thrown
         */
        ODK_NODISCARD PointList getPointListValue() const;

        /**
         * Returns the value of this property as PropertyList
         * If this property is not of type PROPERTY_LIST, an exception is thrown
         */
        ODK_NODISCARD const PropertyList& getPropertyListValue() const;

        virtual pugi::xml_node appendTo(pugi::xml_node parent) const;
        virtual bool readFrom(const pugi::xml_node& tree, const Version& version);

        pugi::xml_node appendValue(pugi::xml_node parent) const;
        bool readValue(pugi::xml_node type_node, const Version& version);

        /**
        * @brief returns a representable string of the value
        */
        ODK_NODISCARD std::string valueToString() const;

    private:
        std::string toXMLType(Type type) const;
        Type fromXMLType(const std::string& xml_type) const;

        bool parseEnum(const pugi::xml_node& type_node);

        void appendScalarNode(pugi::xml_node parent) const;
        Scalar parseScalarNode(const pugi::xml_node& type_node);

        void appendDecoratedNumberNode(pugi::xml_node parent) const;
        DecoratedNumber parseDecoratedNumberNode(const pugi::xml_node& type_node);

        void appendRangeNode(pugi::xml_node parent) const;
        Range parseRangeNode(const pugi::xml_node& type_node);

        void appendPropertyListNode(pugi::xml_node parent) const;
        PropertyList parsePropertyListNode(const pugi::xml_node& type_node, const Version& version);

        void appendDoubleListNode(pugi::xml_node parent) const;
        DoubleList parseDoubleListNode(const pugi::xml_node& type_node);

        void appendStringListNode(pugi::xml_node parent) const;
        StringList parseStringListNode(const pugi::xml_node& type_node);

        void appendPointNode(pugi::xml_node parent) const;
        Point parsePointNode(const pugi::xml_node& type_node);

        void appendPointListNode(pugi::xml_node parent) const;
        PointList parsePointListNode(const pugi::xml_node& type_node);

        void appendRationalNode(pugi::xml_node parent) const;
        Rational parseRationalNode(const pugi::xml_node& type_node);

        void appendChannelIdListNode(pugi::xml_node parent) const;
        ChannelIDList parseChannelIdListNode(const pugi::xml_node& type_node);

        std::string unitToString(const std::string& unit_string) const;

        std::string m_name;
        Type m_type;
        std::string m_enum_type; //enum type or (optional) format specification (e.g. for string)
        //only one of m_string_value or m_value may contain a value
        std::string m_string_value;
        std::shared_ptr<void> m_value;
    };
} // namespace neoncfg

/*
Copyright (c) 2013 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "minko/Common.hpp"
#include "minko/Any.hpp"
#include "minko/Signal.hpp"

namespace minko
{
	namespace data
	{			
		class Provider :
			public std::enable_shared_from_this<Provider>
		{
		public:
			typedef std::shared_ptr<Provider>		Ptr;
			typedef std::shared_ptr<const Provider>	ConstPtr;

        private:
            template <typename T>
            struct is_shared_ptr : std::false_type {};
            template <typename T>
            struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

            template <typename T>
            struct is_weak_ptr : std::false_type {};
            template <typename T>
            struct is_weak_ptr<std::weak_ptr<T>> : std::true_type{};

            template <typename T>
            struct is_valid {
                static const bool value = !std::is_pointer<T>::value && !std::is_reference<T>::value
                    && !is_shared_ptr<T>::value &&!is_weak_ptr<T>::value;
            };

		private:
            std::string                                         _key;
			std::unordered_map<std::string, Any>				_values;
            std::string                                         _uuid;

			std::shared_ptr<Signal<Ptr, const std::string&>>	_propertyAdded;
            std::shared_ptr<Signal<Ptr, const std::string&>>	_propertyChanged;
			std::shared_ptr<Signal<Ptr, const std::string&>>	_propertyRemoved;

		public:
			static
			Ptr
			create(const std::string& key)
			{
				Ptr provider = std::shared_ptr<Provider>(new Provider(key));
				
				return provider;
			}

			static
			Ptr
			create(Ptr source)
			{
				return create(source->_key)->copyFrom(source);
			}

            const std::string&
            key()
            {
                return _key;
            }

            const std::string&
            uuid()
            {
                return _uuid;
            }

			inline
			bool 
            hasProperty(const std::string& name, bool skipPropertyNameFormatting = false) const
            {
                return _values.count(skipPropertyNameFormatting ? name : formatPropertyName(name)) != 0;
            }

			inline
			const std::unordered_map<std::string, Any>&
			values() const
			{
				return _values;
			}

			inline
			std::shared_ptr<Signal<Ptr, const std::string&>>
			propertyAdded() const
			{
				return _propertyAdded;
			}

            inline
			std::shared_ptr<Signal<Ptr, const std::string&>>
			propertyChanged() const
			{
				return _propertyChanged;
			}

			inline
			std::shared_ptr<Signal<Ptr, const std::string&>>
			propertyRemoved() const
			{
				return _propertyRemoved;
			}

			template <typename T>
			inline
            typename std::enable_if<is_valid<T>::value, const T&>::type
            get(const std::string& name, bool skipPropertyNameFormatting = false) const
			{
                return *Any::cast<T>(&_values.at(skipPropertyNameFormatting ? name : formatPropertyName(name)));
			}

            template <typename T>
            inline
            typename std::enable_if<is_valid<T>::value, const T*>::type
            getPointer(const std::string& name, bool skipPropertyNameFormatting = false) const
            {
                return Any::cast<T>(&_values.at(skipPropertyNameFormatting ? name : formatPropertyName(name)));
            }

            template <typename T>
            typename std::enable_if<is_valid<T>::value, Ptr>::type
            set(const std::string& name, T value, bool skipPropertyNameFormatting = false)
            {
                auto formattedPropertyName = skipPropertyNameFormatting ? name : formatPropertyName(name);

                if (_values.count(formattedPropertyName) != 0)
                {
                    T* ptr = Any::cast<T>(&_values[formattedPropertyName]);
                    auto changed = !(*ptr == value);

                    *ptr = value;
                    if (changed)
                        _propertyChanged->execute(shared_from_this(), name);
                }
                else
                {
                    _values[formattedPropertyName] = value;
                    _propertyAdded->execute(shared_from_this(), name);
                    _propertyChanged->execute(shared_from_this(), name);
                }

                return shared_from_this();
            }

            template <typename T>
			bool
			propertyHasType(const std::string& name, bool skipPropertyNameFormatting = false) const
			{
                const std::string&	formattedName   = skipPropertyNameFormatting ? name : formatPropertyName(name);
				const auto			foundIt			= _values.find(formattedName);

				if (foundIt == _values.end())
					throw std::invalid_argument("propertyName");

				try
				{
					Any::cast<T>(foundIt->second);
				}
				catch (...)
				{
					return false;
				}
				
				return true;
			}

			virtual
			Ptr
            unset(const std::string& propertyName, bool skipPropertyNameFormatting = false);

			Ptr
			swap(const std::string& propertyName1,
                 const std::string& propertyName2,
                 bool               skipPropertyNameFormatting = false);

			Ptr
			clone();

			virtual
			Ptr
			copyFrom(Ptr source);

            static
            const std::string
            getActualPropertyName(const std::unordered_map<std::string, std::string>&   variables,
                                  const std::string&                                    propertyName);

		protected:
			Provider(const std::string& key);

			std::string
			formatPropertyName(const std::string& propertyName) const;

			std::string
			unformatPropertyName(const std::string& propertyName) const;
		};
	}
}

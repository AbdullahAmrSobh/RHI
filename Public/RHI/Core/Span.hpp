#pragma once
#include <cstdint>

namespace RHI {

	namespace stl 
	{
		
		template<class T, size_t spanSize>
		class Span 
		{
		public:
			Span(T* data);			
			
			T* begin() const { return m_ptr; }
			T* end() const { return m_ptr + spanSize; }
			
			constexpr size_t size() const { return spanSize; }
			
			T& operator[](size_t index) const { return m_ptr[index]; }
		
		private:
			T* m_ptr;
		};

		template<class T>
		class Span<T, 0>
		{
		public:	
			Span(T* data) : m_ptr(data) {}
			
			T* begin() const { return m_ptr; }
			T* end() const { return m_ptr + m_size; }
			
			constexpr size_t size() const { return 0; }
			
			T& operator[](size_t index) const { return m_ptr[index]; }
		
		private:
			T* m_ptr;
			size_t m_size;		
		};
	}

	

}

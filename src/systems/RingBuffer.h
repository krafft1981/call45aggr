#pragma once

#include <utility>

// Use:
// RingBuffer r;
// r.push(); r.back() = new_element;
// oldest_element = r.front(); r.pop();

template <typename ET, int S, typename ST = unsigned>
class RingBuffer {
  public:
    typedef ET value_type;
    typedef ST size_type;

    RingBuffer() { clear(); }

    ~RingBuffer() {}

    size_type size() const { return m_size; }
    size_type max_size() const { return S; }

    bool empty() const { return m_size == 0; }
    bool full() const { return m_size == S; }

    value_type &front() { return m_buffer[m_front]; }
    value_type &back() { return m_buffer[m_back]; }

    void clear() {
        m_size  = 0;
        m_front = 0;
        m_back  = S - 1;
    }

    void push() {
        m_back = (m_back + 1) % S;
        if (size() == S)
            m_front = (m_front + 1) % S;
        else
            m_size++;
    }

    void push(const value_type &x) {
        push();
        back() = x;
    }

    void push(value_type &&x) {
        push();
        back() = std::move(x);
    }

    void pop() {
        if (m_size > 0) {
            m_size--;
            m_front = (m_front + 1) % S;
        }
    }

    template<typename OutIt>
    void copy_to(OutIt outIt) const {
       size_type idx = m_front;
       size_type counter = 0;
       while (counter < m_size) {
         idx = idx % S;
         *outIt = m_buffer[idx];
         ++idx;
         ++counter;
         ++outIt;
       }
    }

  protected:
    value_type m_buffer[S];

    size_type m_size;

    size_type m_front;
    size_type m_back;
};

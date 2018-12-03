/*
 * RandomExtract.h
 *
 *  Created on: 2013-3-8
 */

#ifndef RANDOMEXTRACT_H_
#define RANDOMEXTRACT_H_

#include <time.h>
#include <exception>
#include <list>
#include <vector>

using namespace std;

template <typename T>
class RandomExtract
{
private:
    typedef vector<T> REPoolVector;
    typedef typename REPoolVector::iterator REPoolVectorIter;

public:
                        RandomExtract()
    {
        m_oExtractPool.clear();
        m_iValidCount= 0;
    }
    virtual             ~RandomExtract() {}

    virtual void        Clear()
    {
        m_oExtractPool.clear();
        m_iValidCount = 0;
    }

    virtual void        InsertElement(T& element)
    {
        m_oExtractPool.push_back(element);
        m_iValidCount++;
    }

    virtual bool        Extract(T& element, int random)
    {
        if ( m_iValidCount <= 0 )
        {
            return false;
        }

        int pos = random % m_iValidCount;
        try
        {
            element = m_oExtractPool.at(pos);
            if ( m_iValidCount > 1 && pos < (m_iValidCount - 1) )
            {
                m_oExtractPool[pos] = m_oExtractPool[m_iValidCount-1];
            }
            m_iValidCount--;
        }
        catch(const std::exception& e)
        {
            return false;
        }

        return true;
    }

private:
    REPoolVector           m_oExtractPool;
    int                   m_iValidCount;
};



#endif /* RANDOMEXTRACT_H_ */

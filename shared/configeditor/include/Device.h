#ifndef DEVICE_H
#define DEVICE_H

#include <string>

using namespace std;

class Device
{
    public:
        Device();
        Device(string type, string id, string name);
        virtual ~Device();
        Device(const Device& other);
        Device& operator=(const Device& other);
        string GetType() { return m_Type; }
        void SetType(string val) { m_Type = val; }
        string GetId() { return m_Id; }
        void SetId(string val) { m_Id = val; }
        string GetName() { return m_Name; }
        void SetName(string val) { m_Name = val; }
    protected:
    private:
        string m_Type;
        string m_Id;
        string m_Name;
};

#endif // DEVICE_H

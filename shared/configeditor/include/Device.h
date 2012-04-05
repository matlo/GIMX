#ifndef DEVICE_H
#define DEVICE_H


class Device
{
    public:
        Device();
        Device(wxString type, wxString id, wxString name);
        virtual ~Device();
        Device(const Device& other);
        Device& operator=(const Device& other);
        wxString GetType() { return m_Type; }
        void SetType(wxString val) { m_Type = val; }
        wxString GetId() { return m_Id; }
        void SetId(wxString val) { m_Id = val; }
        wxString GetName() { return m_Name; }
        void SetName(wxString val) { m_Name = val; }
    protected:
    private:
        wxString m_Type;
        wxString m_Id;
        wxString m_Name;
};

#endif // DEVICE_H

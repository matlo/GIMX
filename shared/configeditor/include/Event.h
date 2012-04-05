#ifndef EVENT_H
#define EVENT_H


class Event
{
    public:
        Event();
        Event(wxString type, wxString id, wxString threshold, wxString deadZone, wxString multiplier, wxString exponent, wxString shape, wxString buffersize, wxString filter);
        Event(wxString type, wxString id, wxString threshold);
        Event(wxString type, wxString id);
        Event(wxString type, wxString id, wxString deadZone, wxString multiplier, wxString exponent, wxString shape, wxString buffersize, wxString filter);
        Event(wxString id);
        virtual ~Event();
        Event(const Event& other);
        Event& operator=(const Event& other);
        wxString GetType() { return m_Type; }
        void SetType(wxString val) { m_Type = val; }
        wxString GetId() { return m_Id; }
        void SetId(wxString val) { m_Id = val; }
        wxString GetThreshold() { return m_Threshold; }
        void SetThreshold(wxString val) { m_Threshold = val; }
        wxString GetDeadZone() { return m_DeadZone; }
        void SetDeadZone(wxString val) { m_DeadZone = val; }
        wxString GetMultiplier() { return m_Multiplier; }
        void SetMultiplier(wxString val) { m_Multiplier = val; }
        wxString GetExponent() { return m_Exponent; }
        void SetExponent(wxString val) { m_Exponent = val; }
        wxString GetShape() { return m_Shape; }
        void SetShape(wxString val) { m_Shape = val; }
        wxString GetBufferSize() { return m_BufferSize; }
        void SetBufferSize(wxString val) { m_BufferSize = val; }
        wxString GetFilter() { return m_Filter; }
        void SetFilter(wxString val) { m_Filter = val; }
    protected:
    private:
        wxString m_Type;
        wxString m_Id;
        wxString m_Threshold;
        wxString m_DeadZone;
        wxString m_Multiplier;
        wxString m_Exponent;
        wxString m_Shape;
        wxString m_BufferSize;
        wxString m_Filter;
};

#endif // EVENT_H

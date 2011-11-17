#ifndef EVENT_CATCHER_H
#define EVENT_CATCHER_H

#define MAX_DEVICES 256

class event_catcher
{
    public:
        event_catcher();
        virtual ~event_catcher();
        wxString GetDeviceType() { return m_DeviceType; }
        wxString GetDeviceName() { return m_DeviceName; }
        wxString GetDeviceId() { return m_DeviceId; }
        wxString GetEventType() { return m_EventType; }
        wxString GetEventId() { return m_EventId; }
        void run();
    protected:
    private:
        void init();
        wxString m_DeviceType;
        wxString m_DeviceName;
        wxString m_DeviceId;
        wxString m_EventType;
        wxString m_EventId;
        unsigned int done;
};

#endif // EVENT_CATCHER_H

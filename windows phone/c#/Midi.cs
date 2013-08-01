using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using Microsoft.Phone.Net.NetworkInformation;
using System.Collections.Generic;
using System.Diagnostics;

namespace DSMI
{
    public delegate void MessageReceivedHandler(MidiMessageReceivedEventArgs e);
    public delegate void ControlChangeReceivedHandler(MidiMessageReceivedEventArgs e);
    public delegate void ProgramChangeReceivedHandler(MidiMessageReceivedEventArgs e);
    public delegate void NoteOnReceivedHandler(MidiMessageReceivedEventArgs e);
    public delegate void NoteOffReceivedHandler(MidiMessageReceivedEventArgs e);
    public delegate void AftertouchReceivedHandler(MidiMessageReceivedEventArgs e);

    /// <summary>
    /// Send and receives MIDI messages from the DSMI server that you can download here :
    /// http://code.google.com/p/dsmi/downloads/list?q=dsmi+server
    /// </summary>
    public static class Midi
    {
        private const short PC_PORT = 9000;
        private const short WP_PORT = 9001;

        /// <summary>
        /// Fired when a MIDI message is received from the server
        /// </summary>
        public static event MessageReceivedHandler MessageReceivedHandler;

        /// <summary>
        /// Fired when a MIDI Control Change message is received from the server
        /// </summary>
        public static event ControlChangeReceivedHandler ControlChangeReceivedHandler;

        /// <summary>
        /// Fired when a MIDI Program Change message is received from the server
        /// </summary>
        public static event ProgramChangeReceivedHandler ProgramChangeReceivedHandler;

        /// <summary>
        /// Fired when a MIDI Note On message is received from the server
        /// </summary>
        public static event NoteOnReceivedHandler NoteOnReceivedHandler;

        /// <summary>
        /// Fired when a MIDI Note Off message is received from the server
        /// </summary>
        public static event NoteOffReceivedHandler NoteOffReceivedHandler;

        /// <summary>
        /// Fired when a MIDI Note Aftertouch message is received from the server
        /// </summary>
        public static event AftertouchReceivedHandler AftertouchReceivedHandler;

        private static Socket _inSocket;
        private static IPEndPoint _inEndpoint;
        private static IPEndPoint _outEndpoint;
        private static Boolean _init = false;

        private static ManualResetEvent _clientDone = new ManualResetEvent(false);

        private static List<Action<byte[]>> _receiveHandlers = new List<Action<byte[]>>();

        /// <summary>
        /// Initialize DSMI for sending and receiving midi messages.
        /// </summary>
        public static void Init()
        {
            if (_init)
                return;

            _inSocket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);

            _inSocket.SetNetworkRequirement(NetworkSelectionCharacteristics.NonCellular);

            _inEndpoint = new IPEndPoint(IPAddress.Any, WP_PORT);
            _outEndpoint = new IPEndPoint(IPAddress.Broadcast, PC_PORT);

            _inSocket.Bind(_inEndpoint);

            _init = true;

            _clientDone.Reset();
            SendKeepAlive();
            _clientDone.WaitOne(100);

            Receive();
        }

        /// <summary>
        /// Sends a keepalive message (0, 0, 0) to the server.
        /// </summary>
        public static void SendKeepAlive()
        {
            Send(new MidiMessage(0,0,0));
        }

        private static Boolean NetworkAvailable
        {
            get
            {
                if (!_init)
                    return false;

                if (!NetworkInterface.GetIsNetworkAvailable())
                    return false;

                if (_inSocket == null)
                    return false;

                return true;
            }
        }

        private static SocketAsyncEventArgs GetEventArgs(IPEndPoint endpoint, byte[] buffer)
        {
            SocketAsyncEventArgs eventArgs = new SocketAsyncEventArgs();
            eventArgs.RemoteEndPoint = endpoint;
            eventArgs.SetBuffer(buffer, 0, buffer.Length);
            return eventArgs;
        }

        private static byte[] BuildMessage(byte byte1, byte byte2, byte byte3)
        {
            byte[] message = new byte[3];

            message[0] = byte1;
            message[1] = byte2;
            message[2] = byte3;

            return message;
        }

        private static void Receive()
        {
            if (!NetworkAvailable)
                return;

            SocketAsyncEventArgs eventArgs = GetEventArgs(_outEndpoint, new Byte[3]);
            
            eventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(delegate(object s, SocketAsyncEventArgs e)
            {
                if (e.SocketError == SocketError.Success && e.BytesTransferred == 3)
                {
                    Debug.WriteLine("DSMI: Midi Message Received {0} {1} {2}", e.Buffer[0], e.Buffer[1], e.Buffer[2]);

                    byte message = e.Buffer[0];
                    byte data1 = e.Buffer[1];
                    byte data2 = e.Buffer[2];
                    byte channel = (byte)(message & 0x0F);

                    MidiMessage midiMessage;

                    if ((message & MidiMessageType.MIDI_CC) == MidiMessageType.MIDI_CC)
                    {
                        midiMessage = new MidiControlChange(channel, data1, data2);

                        if (ControlChangeReceivedHandler != null)
                            ControlChangeReceivedHandler(new MidiMessageReceivedEventArgs(midiMessage));
                    }
                    else if ((message & MidiMessageType.MIDI_PC) == MidiMessageType.MIDI_PC)
                    {
                        midiMessage = new MidiProgramChange(channel, data1);

                        if (ProgramChangeReceivedHandler != null)
                            ProgramChangeReceivedHandler(new MidiMessageReceivedEventArgs(midiMessage));
                    }
                    else if ((message & MidiMessageType.NOTE_ON) == MidiMessageType.NOTE_ON)
                    {
                        midiMessage = new MidiNoteOn(channel, data1, data2);

                        if (NoteOnReceivedHandler != null)
                            NoteOnReceivedHandler(new MidiMessageReceivedEventArgs(midiMessage));
                    }
                    else if ((message & MidiMessageType.NOTE_OFF) == MidiMessageType.NOTE_OFF)
                    {
                        midiMessage = new MidiNoteOff(channel, data1, data2);

                        if (NoteOffReceivedHandler != null)
                            NoteOffReceivedHandler(new MidiMessageReceivedEventArgs(midiMessage));
                    }
                    else if ((message & MidiMessageType.MIDI_AT) == MidiMessageType.MIDI_AT)
                    {
                        midiMessage = new MidiAftertouch(channel, data1, data2);

                        if (AftertouchReceivedHandler != null)
                            AftertouchReceivedHandler(new MidiMessageReceivedEventArgs(midiMessage));
                    }
                    else
                    {
                        midiMessage = new MidiMessage(message, data1, data2);
                    }

                    if (MessageReceivedHandler != null)
                        MessageReceivedHandler(new MidiMessageReceivedEventArgs(midiMessage));
                }
                Receive();
            });
            try
            {
                _inSocket.ReceiveFromAsync(eventArgs);
            }
            catch
            {
                _inSocket.Bind(_inEndpoint);
                Receive();
            }
        }

        /// <summary>
        /// Sends a MIDI message to the DSMI server
        /// </summary>
        /// <param name="midiMessage">The MIDI message to send</param>
        public static void Send(MidiMessage midiMessage)
        {
            if (!NetworkAvailable)
                return;

            byte[] toSend = BuildMessage(midiMessage.Message, midiMessage.Data1, midiMessage.Data2);

            SocketAsyncEventArgs eventArgs = GetEventArgs(_outEndpoint, toSend);

            eventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(delegate(object s, SocketAsyncEventArgs e)
            {
                Debug.WriteLine("DSMI: Midi Message Sent {0} {1} {2}", midiMessage.Message, midiMessage.Data1, midiMessage.Data2);

                _clientDone.Set();
            });

            Socket.ConnectAsync(SocketType.Dgram, ProtocolType.Udp, eventArgs);
        }

        public static void SendAllNotesOff(byte channel)
        {
            Send(new MidiControlChange(channel, 123, 0));
        }
    }

}

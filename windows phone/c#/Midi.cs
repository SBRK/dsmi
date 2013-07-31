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
    public delegate void CCReceivedHandler(MidiCC message);
    public delegate void NoteOnReceivedHandler(MidiNoteOn message);
    public delegate void NoteOffReceivedHandler(MidiNoteOff message);
    public delegate void NoteAftertouchReceivedHandler(MidiNoteAftertouch message);
    public delegate void PCReceivedHandler(MidiPC message);

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
        public static event CCReceivedHandler CCReceivedHandler;

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
        public static event NoteAftertouchReceivedHandler NoteAftertouchReceivedHandler;

        /// <summary>
        /// Fired when a MIDI Program Change message is received from the server
        /// </summary>
        public static event PCReceivedHandler PCReceivedHandler;

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
            Send(0, 0, 0);
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

                    if (MidiMessageReceivedHandler != null)
                        MidiMessageReceivedHandler(new MidiMessageReceivedEventArgs(message, data1, data2));

                    if (MidiCCReceivedHandler != null && (message & MidiMessageType.MIDI_CC) == MidiMessageType.MIDI_CC)
                        MidiCCReceivedHandler(new MidiCC(channel, data1, data2));
                    else if (MidiNoteOnReceivedHandler != null && (message & MidiMessageType.NOTE_ON) == MidiMessageType.NOTE_ON)
                        MidiNoteOnReceivedHandler(new MidiNoteOn(channel, data1, data2));
                    else if (MidiNoteOffReceivedHandler != null && (message & MidiMessageType.NOTE_OFF) == MidiMessageType.NOTE_OFF)
                        MidiNoteOffReceivedHandler(new MidiNoteOff(channel, data1, data2));
                    else if (MidiNoteAftertouchReceivedHandler != null && (message & MidiMessageType.MIDI_AT) == MidiMessageType.MIDI_AT)
                        MidiNoteAftertouchReceivedHandler(new MidiNoteAftertouch(channel, data1, data2));
                    else if (MidiPCReceivedHandler != null && (message & MidiMessageType.MIDI_PC) == MidiMessageType.MIDI_PC)
                        MidiPCReceivedHandler(new MidiPC(channel, data1));
                }
                Receive();
            });
            _inSocket.ReceiveFromAsync(eventArgs);
        }

        /// <summary>
        /// Sends a MIDI message to the DSMI server
        /// </summary>
        /// <param name="message">MIDI message type</param>
        /// <param name="data1">Data 1 of the message</param>
        /// <param name="data2">Data 2 of the message</param>
        public static void Send(byte message, byte data1, byte data2)
        {
            if (!NetworkAvailable)
                return;

            byte[] toSend = BuildMessage(message, data1, data2);

            SocketAsyncEventArgs eventArgs = GetEventArgs(_outEndpoint, toSend);

            eventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(delegate(object s, SocketAsyncEventArgs e)
            {
                Debug.WriteLine("DSMI: Midi Message Sent {0} {1} {2}", message, data1, data2);

                _clientDone.Set();
            });

            Socket.ConnectAsync(SocketType.Dgram, ProtocolType.Udp, eventArgs);
        }

        public static void Send(MidiCC MidiCC)
        {
            Send((byte)(MidiMessageType.MIDI_CC | MidiCC.Channel), MidiCC.ControlNumber, MidiCC.Value);
        }

        public static void Send(MidiNoteOn MidiNoteOn)
        {
            Send((byte)(MidiMessageType.NOTE_ON | MidiNoteOn.Channel), MidiNoteOn.NoteNumber, MidiNoteOn.Velocity);
        }

        public static void Send(MidiNoteOff MidiNoteOff)
        {
            Send((byte)(MidiMessageType.NOTE_OFF | MidiNoteOff.Channel), MidiNoteOff.NoteNumber, MidiNoteOff.Velocity);
        }

        public static void Send(MidiNoteAftertouch MidiNoteAftertouch)
        {
            Send((byte)(MidiMessageType.MIDI_AT | MidiNoteAftertouch.Channel), MidiNoteAftertouch.NoteNumber, MidiNoteAftertouch.Pressure);
        }

        public static void Send(MidiPC MidiPC)
        {
            Send((byte)(MidiMessageType.MIDI_CC | MidiPC.Channel), MidiPC.ProgramNumber, 0);
        }

        public static void SendAllNotesOff(byte channel)
        {
            Send(new MidiCC(channel, 123, 0));
        }
    }

}

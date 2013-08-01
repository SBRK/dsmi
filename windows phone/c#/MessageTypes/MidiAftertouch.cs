using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public class MidiAftertouch : MidiMessage
    {

        public byte Channel
        {
            get
            {
                return (byte)(Message & 0x0F);
            }
            set
            {
                Message = (byte)(MidiMessageType.MIDI_AT | (value & 0x0F));
            }
        }

        public byte NoteNumber
        {
            get
            {
                return Data1;
            }
            set
            {
                Data1 = value;
            }
        }

        public byte Pressure
        {
            get
            {
                return Data2;
            }
            set
            {
                Data2 = value;
            }
        }


        public MidiAftertouch(byte channel, byte noteNumber, byte pressure)
            : base((byte)(MidiMessageType.MIDI_AT | (channel & 0x0F)), noteNumber, pressure)
        {}
    }
}

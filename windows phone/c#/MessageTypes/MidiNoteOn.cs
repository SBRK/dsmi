using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public class MidiNoteOn : MidiMessage
    {
        public byte Channel
        {
            get
            {
                return (byte)(Message & 0x0F);
            }
            set
            {
                Message = (byte)(MidiMessageType.NOTE_ON | (value & 0x0F));
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

        public byte Velocity
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


        public MidiNoteOn(byte channel, byte noteNumber, byte velocity) 
            : base((byte)(MidiMessageType.NOTE_ON | (channel & 0x0F)), noteNumber, velocity)
        {}
    }
}

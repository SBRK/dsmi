using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public class MidiNoteAftertouch
    {
        public byte Channel { get; set; }
        public byte NoteNumber { get; set; }
        public byte Pressure { get; set; }

        public MidiNoteAftertouch(byte channel, byte noteNumber, byte pressure)
        {
            Channel = channel;
            NoteNumber = noteNumber;
            Pressure = pressure;
        }
    }
}

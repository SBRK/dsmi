using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DSMI
{
    public class MidiMessageType
    {
        /// <summary>
        /// Note On (data1: Note Number, data2: Velocity)
        /// </summary>
        public const byte NOTE_ON = 0x90;

        /// <summary>
        /// Note Off (data1: Note Number, data2: Velocity)
        /// </summary>
        public const byte NOTE_OFF = 0x80;

        /// <summary>
        /// Aftertouch (data1: Note Number, data2: Pressure)
        /// </summary>
        public const byte MIDI_AT = 0xA0;

        /// <summary>
        /// Control Change (data1: Controller Number, data2: Value)
        /// </summary>
        public const byte MIDI_CC = 0xB0;

        /// <summary>
        /// Program Change (data1: Program Number, data2: -)
        /// </summary>
        public const byte MIDI_PC = 0xC0;

        /// <summary>
        /// Pitch Wheel  (data1: Low Byte, data2: High Byte)
        /// </summary>
        public const byte PITCH_WHEEL = 0xE0;

        /// <summary>
        /// MTC (Midi Time Code) Quarter Frame
        /// </summary>
        public const byte MTC_QUARTER_FRAME = 0xF1;

        /// <summary>
        /// Song Position
        /// </summary>
        public const byte SONG_POSITION = 0xF2;

        /// <summary>
        /// Song Select
        /// </summary>
        public const byte SONG_SELECT = 0xF3;

        /// <summary>
        /// Tune Request
        /// </summary>
        public const byte TUNE_REQUEST = 0xF6;

        /// <summary>
        /// Midi Clock
        /// </summary>
        public const byte MIDI_CLOCK = 0xF8;

        /// <summary>
        /// Midi Tick
        /// </summary>
        public const byte MIDI_TICK = 0xF9;

        /// <summary>
        /// Midi Start
        /// </summary>
        public const byte MIDI_START = 0xFA;

        /// <summary>
        /// Midi Stop
        /// </summary>
        public const byte MIDI_STOP = 0xFC;

        /// <summary>
        /// Midi Continue
        /// </summary>
        public const byte MIDI_CONTINUE = 0xFB;

        /// <summary>
        /// Midi Active Sense
        /// </summary>
        public const byte MIDI_ACTIVE_SENSE = 0xFE;

        /// <summary>
        /// Reset
        /// </summary>
        public const byte RESET = 0xFF;
    }
}

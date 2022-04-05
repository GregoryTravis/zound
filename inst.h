/* $Id: inst.h,v 1.1 1998/06/30 02:56:11 mito Exp $ */

#ifndef _inst_h_
#define _inst_h_

/* pseudo-instruments */

#define INST_NOTE(note)       (0x4000+(note)) // 127 of them
#define INST_CONTROLLER(cnum) (0x4100+(cnum)) // 127 of them
#define INST_PITCHWHEEL       (0x5000)        // 1 of them
#define INST_AFTERTOUCH       (0x5100)        // 1 of them
#define INST_CHANNELPRESSURE  (0x5200)        // 1 of them
#define INST_START (INSTPS_MIDI(0))
#define INST_END (INST_CHANNELPRESSURE+1)

#define INST_ISNOTE(inst)            (((inst)&0xFF00)==0x4000)
#define INST_WHICHNOTE(inst)         ((inst)&0x7F)
#define INST_ISCONTROLLER(inst)      (((inst)&0xFF00)==0x4100)
#define INST_WHICHCONTROLLER(inst)   ((inst)&0x7F)
#define INST_ISPITCHWHEEL(inst)      ((inst)==0x5000)
#define INST_ISAFTERTOUCH(inst)      ((inst)==0x5100)
#define INST_ISCHANNELPRESSURE(inst) ((inst)==0x5200)

#define INST_ISMIDI(sev) \
  ((sev)->inst >= INST_START && (sev)->inst < INST_END)

#endif /* _inst_h_ */

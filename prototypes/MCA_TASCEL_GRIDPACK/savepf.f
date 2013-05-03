      SUBROUTINE SAVEPF 
!
! WRITTEN BY YOUSU CHEN, PNNL, 03 JAN 2013
!
!------------DESCRIPTION------------------
! SAEV BASE CASE POWER FLOW VARIABLES
! 
!
      USE BUSMODULE
      USE GENMODULE
      USE BRCHMODULE
      USE YBUSMODULE
      USE OTHERMODULE
      USE CAMODULE
      USE CLOCK
      USE MPI
      USE DEFDP
      USE FLAGS

      IMPLICIT NONE

      NPVSAVE=NPV
      NPQSAVE=NPQ

!
!     ALLOCATE MEMORY FOR PVSAVE AN PQSAVE
!
      IF (ALLOCATED(PVSAVE)) DEALLOCATE(PVSAVE)
      IF (ALLOCATED(PQSAVE)) DEALLOCATE(PQSAVE)
      ALLOCATE(PVSAVE(NPV),PQSAVE(NPQ),STAT=ERROR)
      IF (ERROR /=0) THEN
          PRINT *, " ------------------ ERROR ------------------------"
          PRINT *, " PROGRAM COULD NOT ALLOCATE SPACE FOR PVSAVE ARRAYS"
          STOP
      END IF
      PVSAVE=PV
      PQSAVE=PQ

      IF (ALLOCATED(GBUSSAVE)) DEALLOCATE(GBUSSAVE)
      IF (ALLOCATED(GIDSAVE)) DEALLOCATE(GIDSAVE)
      IF (ALLOCATED(GINDSAVE)) DEALLOCATE(GINDSAVE)

      ALLOCATE(GBUSSAVE(NGON),GIDSAVE(NGON),GINDSAVE(NGON),
     &         STAT=ERROR)
      IF (ERROR /=0) THEN
         PRINT *," ------------------- ERROR --------------------------"
         PRINT *," PROGRAM COULD NOT ALLOCATE SPACE FOR GBUSSAVE ARRAYS"
         STOP
      END IF

      GBUSSAVE=GBUS
      GIDSAVE=GID
      GINDSAVE=GIND
      NGONSAVE=NGON
!      V0SAVE=V0
      VALYBUSSAVE=VALYBUS
      IF (ALLOCATED(PGGSAVE)) DEALLOCATE(PGGSAVE)
      IF (ALLOCATED(QGGSAVE)) DEALLOCATE(QGGSAVE)
      ALLOCATE(PGGSAVE(NGON),QGGSAVE(NGON), STAT=ERROR)
      IF (ERROR /=0) THEN
         PRINT *," ------------------- ERROR --------------------------"
         PRINT *," PROGRAM COULD NOT ALLOCATE SPACE FOR PGGSAVE ARRAYS"
         STOP
      END IF
      SBUSSAVE=SBUS
      PGGSAVE=PGG
      QGGSAVE=QGG
      VMSAVE=VM
      VASAVE=VA
      PFSAVE=PF
      QFSAVE=QF
      PTSAVE=PT
      QTSAVE=QT

      ENDSUBROUTINE
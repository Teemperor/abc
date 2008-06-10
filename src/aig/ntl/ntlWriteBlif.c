/**CFile****************************************************************

  FileName    [ntlWriteBlif.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Command processing package.]

  Synopsis    [Procedures to write BLIF files.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: ntlWriteBlif.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "ntl.h"
#include "ioa.h"

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////
 
/**Function*************************************************************

  Synopsis    [Writes one model into the BLIF file.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ioa_WriteBlifModel( FILE * pFile, Ntl_Mod_t * pModel, int fMain )
{
    Ntl_Obj_t * pObj;
    Ntl_Net_t * pNet;
    float Delay;
    int i, k;
    fprintf( pFile, ".model %s\n", pModel->pName );
    if ( pModel->attrWhite || pModel->attrBox || pModel->attrComb || pModel->attrKeep )
    {
        fprintf( pFile, ".attrib" );
        fprintf( pFile, " %s", pModel->attrWhite? "white": "black" );
        fprintf( pFile, " %s", pModel->attrBox?   "box"  : "logic" );
        fprintf( pFile, " %s", pModel->attrComb?  "comb" : "seq"   );
//        fprintf( pFile, " %s", pModel->attrKeep?  "keep" : "sweep" );
        fprintf( pFile, "\n" );
    }
    fprintf( pFile, ".inputs" );
    Ntl_ModelForEachPi( pModel, pObj, i )
        fprintf( pFile, " %s", Ntl_ObjFanout0(pObj)->pName );
    fprintf( pFile, "\n" );
    fprintf( pFile, ".outputs" );
    Ntl_ModelForEachPo( pModel, pObj, i )
        fprintf( pFile, " %s", Ntl_ObjFanin0(pObj)->pName );
    fprintf( pFile, "\n" );
    // write delays
    if ( pModel->vDelays )
    {
        for ( i = 0; i < Vec_IntSize(pModel->vDelays); i += 3 )
        {
            fprintf( pFile, ".delay" );
            if ( Vec_IntEntry(pModel->vDelays,i) != -1 )
                fprintf( pFile, " %s", Ntl_ObjFanout0(Ntl_ModelPi(pModel, Vec_IntEntry(pModel->vDelays,i)))->pName );
            if ( Vec_IntEntry(pModel->vDelays,i+1) != -1 )
                fprintf( pFile, " %s", Ntl_ObjFanin0(Ntl_ModelPo(pModel, Vec_IntEntry(pModel->vDelays,i+1)))->pName );
            fprintf( pFile, " %.3f", Aig_Int2Float(Vec_IntEntry(pModel->vDelays,i+2)) );
            fprintf( pFile, "\n" );
        }
    }
    if ( pModel->vTimeInputs )
    {
        for ( i = 0; i < Vec_IntSize(pModel->vTimeInputs); i += 2 )
        {
            if ( fMain )
                fprintf( pFile, ".input_arrival" );
            else
                fprintf( pFile, ".input_required" );
            if ( Vec_IntEntry(pModel->vTimeInputs,i) != -1 )
                fprintf( pFile, " %s", Ntl_ObjFanout0(Ntl_ModelPi(pModel, Vec_IntEntry(pModel->vTimeInputs,i)))->pName );
            Delay = Aig_Int2Float(Vec_IntEntry(pModel->vTimeInputs,i+1));
            if ( Delay == -TIM_ETERNITY )
                fprintf( pFile, " -inf" );
            else if ( Delay == TIM_ETERNITY )
                fprintf( pFile, " inf" );
            else
                fprintf( pFile, " %.3f", Delay );
            fprintf( pFile, "\n" );
        }
    }
    if ( pModel->vTimeOutputs )
    {
        for ( i = 0; i < Vec_IntSize(pModel->vTimeOutputs); i += 2 )
        {
            if ( fMain )
                fprintf( pFile, ".output_required" );
            else
                fprintf( pFile, ".output_arrival" );
            if ( Vec_IntEntry(pModel->vTimeOutputs,i) != -1 )
                fprintf( pFile, " %s", Ntl_ObjFanin0(Ntl_ModelPo(pModel, Vec_IntEntry(pModel->vTimeOutputs,i)))->pName );
            Delay = Aig_Int2Float(Vec_IntEntry(pModel->vTimeOutputs,i+1));
            if ( Delay == -TIM_ETERNITY )
                fprintf( pFile, " -inf" );
            else if ( Delay == TIM_ETERNITY )
                fprintf( pFile, " inf" );
            else
                fprintf( pFile, " %.3f", Delay );
            fprintf( pFile, "\n" );
        }
    }
    // write objects
    Ntl_ModelForEachObj( pModel, pObj, i )
    {
        if ( Ntl_ObjIsNode(pObj) )
        {
            fprintf( pFile, ".names" );
            Ntl_ObjForEachFanin( pObj, pNet, k )
                fprintf( pFile, " %s", pNet->pName );
            fprintf( pFile, " %s\n", Ntl_ObjFanout0(pObj)->pName );
            fprintf( pFile, "%s", pObj->pSop );
        }
        else if ( Ntl_ObjIsLatch(pObj) )
        {
            fprintf( pFile, ".latch" );
            fprintf( pFile, " %s", Ntl_ObjFanin0(pObj)->pName );
            fprintf( pFile, " %s", Ntl_ObjFanout0(pObj)->pName );
            assert( pObj->LatchId.regType == 0 || pObj->LatchId.regClass == 0 );
            if ( pObj->LatchId.regType )
            {
                if ( pObj->LatchId.regType == 1 )
                    fprintf( pFile, " fe" );
                else if ( pObj->LatchId.regType == 2 )
                    fprintf( pFile, " re" );
                else if ( pObj->LatchId.regType == 3 )
                    fprintf( pFile, " ah" );
                else if ( pObj->LatchId.regType == 4 )
                    fprintf( pFile, " al" );
                else if ( pObj->LatchId.regType == 5 )
                    fprintf( pFile, " as" );
                else
                    assert( 0 );
            }
            else if ( pObj->LatchId.regClass )
                fprintf( pFile, " %d", pObj->LatchId.regClass );
            if ( pObj->pClock )
                fprintf( pFile, " %s", pObj->pClock->pName );
            fprintf( pFile, " %d", pObj->LatchId.regInit );
            fprintf( pFile, "\n" );
        }
        else if ( Ntl_ObjIsBox(pObj) )
        {
            fprintf( pFile, ".subckt %s", pObj->pImplem->pName );
            Ntl_ObjForEachFanin( pObj, pNet, k )
                fprintf( pFile, " %s=%s", Ntl_ModelPiName(pObj->pImplem, k), pNet->pName );
            Ntl_ObjForEachFanout( pObj, pNet, k )
                fprintf( pFile, " %s=%s", Ntl_ModelPoName(pObj->pImplem, k), pNet->pName );
            fprintf( pFile, "\n" );
        }
    }
    fprintf( pFile, ".end\n\n" );
}

/**Function*************************************************************

  Synopsis    [Writes the netlist into the BLIF file.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ioa_WriteBlif( Ntl_Man_t * p, char * pFileName )
{
    FILE * pFile;
    Ntl_Mod_t * pModel;
    int i;
    // start the output stream
    pFile = fopen( pFileName, "w" );
    if ( pFile == NULL )
    {
        fprintf( stdout, "Ioa_WriteBlif(): Cannot open the output file \"%s\".\n", pFileName );
        return;
    }
    fprintf( pFile, "# Benchmark \"%s\" written by ABC-8 on %s\n", p->pName, Ioa_TimeStamp() );
    // write the models
    Ntl_ManForEachModel( p, pModel, i )
        Ioa_WriteBlifModel( pFile, pModel, i==0 );
    // close the file
    fclose( pFile );
}

/**Function*************************************************************

  Synopsis    [Writes the logic network into the BLIF file.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ioa_WriteBlifLogic( Nwk_Man_t * pNtk, Ntl_Man_t * p, char * pFileName )
{
    Ntl_Man_t * pNew;
    pNew = Ntl_ManInsertNtk( p, pNtk );
    Ioa_WriteBlif( pNew, pFileName );
    Ntl_ManFree( pNew );
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////



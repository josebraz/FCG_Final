#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "obj_model.h"

// Classe que engloba v�rias fun��es utils para debbug
// Estra�da de fun��es da Main nos Labs
class debugger
{
    public:
        debugger();
        virtual ~debugger();

        void PrintObjModelInfo(ObjModel* model);

    protected:

    private:
};

#endif // DEBUGGER_H

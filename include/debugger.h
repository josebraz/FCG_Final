#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "obj_model.h"

// Classe que engloba várias funções utils para debbug
// Estraída de funções da Main nos Labs
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

#pragma once

namespace Neko
{
    namespace Nova
    {
        // @todo
        class DatabaseContext
        {
        public:
            
            DatabaseContext();
            
            virtual ~DatabaseContext();
            
            void SetTransactionEnabled(bool enabled);
            
            void BeginTransaction(void* database);
            void CommitTransactions();
            
            void RollbackTransactions();
            
            void Release();
        };
    }
}

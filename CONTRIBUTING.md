### Repo Structure
```
# Entity Definitions
domain/                    # This is the core directory where the domains are defined.
  abstracts/               # Abstract base classes for common attributes and methods.
  models/                  # Domain interface definitions, e.g. model, assistant.
  repositories/            # Extensions abstract and interface

# Business Rules
usecases/                  # Application logic 
	assistants/        # CRUD logic (invokes dtos, entities).
	chat/              # Logic for chat functionalities.
	models/            # Logic for model operations.

# Adapters & Implementations
infrastructure/            # Implementations for Cortex interactions
  commanders/              # CLI handlers
    models/
    questions/             # CLI installation UX
    shortcuts/             # CLI chained syntax
    types/
    usecases/              # Invokes UseCases

  controllers/             # Nest controllers and HTTP routes
    assistants/		   # Invokes UseCases
    chat/     		   # Invokes UseCases
    models/                # Invokes UseCases
	
  database/                # Database providers (mysql, sqlite)
	
# Framework specific object definitions
  dtos/                    # DTO definitions (data transfer & validation)
  entities/                # TypeORM entity definitions (db schema)
  
# Providers
  providers/cortex         # Cortex [server] provider (a core extension)
  repositories/extensions  # Extension provider (core & external extensions)

extensions/                # External extensions
command.module.ts          # CLI Commands List
main.ts                    # Entrypoint
```

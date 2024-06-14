type Constructor = new (...args: any[]) => any;

export const SetCommandContext = () => {
  return (constructor: Constructor) => {
    const classMethods = Object.getOwnPropertyNames(constructor.prototype);

    classMethods.forEach((methodName) => {
      if (methodName !== 'run') return;
      const originalMethod = constructor.prototype[methodName];
      if (typeof originalMethod === 'function') {
        constructor.prototype[methodName] = async function (...args: any[]) {
          if (this.contextService) {
            const parentCommandName = this.command.parent
              ? this.command.parent.name()
              : '';
            const comamndName = this.command.name();
            this.contextService.set(
              'command',
              `${parentCommandName} ${comamndName}`,
            );
            if (parentCommandName === 'models') {
              const modelId = args[0]?.[0];
              if (modelId) {
                this.contextService.set('model', modelId);
              }
            }
            // const modelId = args[1].model
          }
          const result = await originalMethod.apply(this, args);
          return result;
        };
      }
    });
  };
};

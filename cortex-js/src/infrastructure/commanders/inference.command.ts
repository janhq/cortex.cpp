import { CommandRunner, SubCommand } from 'nest-commander';

// TODO: might need to change the name of the command. Inference seems a bit hard to type lol
@SubCommand({ name: 'inference' })
export class InferenceCommand extends CommandRunner {
  constructor() {
    super();
  }

  async run(_input: string[]): Promise<void> {
    const lineByLine = require('readline');
    const lbl = lineByLine.createInterface({
      input: process.stdin,
      output: process.stdout,
    });
    lbl.on('line', (userInput: string) => {
      if (userInput.trim() === 'exit()') {
        lbl.close();
        return;
      }

      console.log('Result:', userInput);
      console.log('Enter another equation or type "exit()" to quit.');
    });
  }
}

import { CommandRunner, InquirerService, SubCommand } from 'nest-commander';
import { exit } from 'node:process';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';
import { RepoDesignation, listFiles } from '@huggingface/hub';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';

@SubCommand({
  name: 'pull',
  aliases: ['download'],
  description: 'Download a model. Working with HuggingFace model id.',
})
export class ModelPullCommand extends CommandRunner {
  private janHqModelPrefix = 'janhq';

  constructor(
    private readonly inquirerService: InquirerService,
    private readonly modelsCliUsecases: ModelsCliUsecases,
  ) {
    super();
  }

  async run(input: string[]) {
    if (input.length < 1) {
      console.error('Model Id is required');
      exit(1);
    }

    const branches = /[:/]/.test(input[0])
      ? undefined
      : await this.tryToGetBranches(input[0]);

    await this.modelsCliUsecases
      .pullModel(
        !branches ? input[0] : await this.handleJanHqModel(input[0], branches),
      )
      .catch((e: Error) => {
        if (e instanceof ModelNotFoundException)
          console.error('Model does not exist.');
        else console.error(e);
        exit(1);
      });

    console.log('\nDownload complete!');
    exit(0);
  }

  private async tryToGetBranches(input: string): Promise<any> {
    try {
      // try to append with janhq/ if it's not already
      const sanitizedInput = input.trim().startsWith(this.janHqModelPrefix)
        ? input
        : `${this.janHqModelPrefix}/${input}`;

      const repo: RepoDesignation = {
        type: 'model',
        name: sanitizedInput,
      };

      // eslint-disable-next-line @typescript-eslint/no-unused-vars
      for await (const _fileInfo of listFiles({ repo })) {
        break;
      }

      const response = await fetch(
        `https://huggingface.co/api/models/${sanitizedInput}/refs`,
      );
      const data = await response.json();
      const branches: string[] = data.branches.map((branch: any) => {
        return branch.name;
      });

      return branches;
    } catch (err) {
      return undefined;
    }
  }

  private async versionInquiry(tags: string[]): Promise<string> {
    const { tag } = await this.inquirerService.inquirer.prompt({
      type: 'list',
      name: 'tag',
      message: 'Select version',
      choices: tags,
    });

    return tag;
  }

  private async handleJanHqModel(repoName: string, branches: string[]) {
    let selectedTag = branches[0];

    if (branches.length > 1) {
      selectedTag = await this.versionInquiry(branches);
    }

    const revision = selectedTag;
    if (!revision) {
      console.error("Can't find model revision.");
      exit(1);
    }
    // Return parsed model Id
    return `${repoName}:${revision}`;
  }
}

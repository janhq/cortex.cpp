import { CommandRunner, InquirerService, SubCommand } from 'nest-commander';
import { exit } from 'node:process';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';
import { RepoDesignation, listFiles } from '@huggingface/hub';
import { basename } from 'node:path';

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

    const branches = await this.tryToGetBranches(input[0]);

    if (!branches) {
      await this.modelsCliUsecases.pullModel(input[0]);
    } else {
      // if there's metadata.yaml file, we assumed it's a JanHQ model
      await this.handleJanHqModel(input[0], branches);
    }

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
    const sanitizedRepoName = repoName.trim().startsWith(this.janHqModelPrefix)
      ? repoName
      : `${this.janHqModelPrefix}/${repoName}`;

    let selectedTag = branches[0];

    if (branches.length > 1) {
      selectedTag = await this.versionInquiry(branches);
    }

    const revision = selectedTag;
    if (!revision) {
      console.error("Can't find model revision.");
      exit(1);
    }

    const repo: RepoDesignation = { type: 'model', name: sanitizedRepoName };
    let ggufUrl: string | undefined = undefined;
    let fileSize = 0;
    for await (const fileInfo of listFiles({
      repo: repo,
      revision: revision,
    })) {
      if (fileInfo.path.endsWith('.gguf')) {
        ggufUrl = `https://huggingface.co/${sanitizedRepoName}/resolve/${revision}/${fileInfo.path}`;
        fileSize = fileInfo.size;
        break;
      }
    }

    if (!ggufUrl) {
      console.error("Can't find model file.");
      exit(1);
    }
    console.log('Downloading', basename(ggufUrl));
    await this.modelsCliUsecases.pullModelWithExactUrl(
      `${sanitizedRepoName}/${revision}`,
      ggufUrl,
      fileSize,
    );
  }
}

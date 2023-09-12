FROM nitro_dev AS install_neovim_zsh

WORKDIR /code

# Include tmux and github cli for better devex , ripgrep for the telescope plugin
RUN apt update && apt install -qqy zsh unzip gettext build-essential tmux gh ripgrep \
 && apt install -qqy -t llvm-toolchain-bookworm-16 clangd-16 clang-format-16 lldb-16 \
 && for f in /usr/lib/llvm-16/bin/*; do ln -sf "$f" /usr/bin; done \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*


RUN git clone --recursive https://github.com/tikikun/my_container

# neovim install
WORKDIR /code

RUN git clone https://github.com/neovim/neovim
WORKDIR /code/neovim
RUN git checkout v0.9.2
RUN make -j $(nproc) CMAKE_BUILD_TYPE=RelWithDebInfo
RUN make install
WORKDIR /code
RUN rm -rf neovim

FROM install_neovim_zsh as install_cli

RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"


RUN git clone https://github.com/zsh-users/zsh-autosuggestions ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-autosuggestions

RUN git clone https://github.com/z-shell/F-Sy-H.git ${ZSH_CUSTOM:-$HOME/.oh-my-zsh/custom}/plugins/F-Sy-H
# set up container config
WORKDIR /code/my_container/

RUN mkdir -p /root/.config/nvim
RUN cp -r ./vim_setup/* /root/.config/nvim
RUN cp dotfiles/.zshrc /root/.zshrc
RUN cp dotfiles/.tmux.conf /root/.tmux.conf

# Set working directory
WORKDIR /workspace/workdir

# Set the entrypoint script
ENTRYPOINT [ "zsh"]




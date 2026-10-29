# =====================================================================
# project_A —— C++ / Python 混合开发工程入口
#
# 常用命令：
#   make            # 同 make help，列出所有目标
#   make build      # 增量编译 C++ 扩展模块（build/ 下）
#   make test       # 激活 venv 并运行 Python 测试
#   make valgrind   # 编译 C++ 测试并做内存泄漏检测
#   make clean      # 清理 build/ 下的构建产物
#   make diagnose   # 一键诊断（= make valgrind + make test，等价 bash diagnose.sh）
# =====================================================================

SHELL := /bin/bash
.DEFAULT_GOAL := help

PROJECT_ROOT := $(CURDIR)
BUILD_DIR    := build
VENV_DIR     := .venv
VENV_ACTIVATE := source $(VENV_DIR)/bin/activate
PYTHON       := $(VENV_DIR)/bin/python

CXX            ?= g++
CXX_TEST_SRC   := tests/test_cpp.cpp
CXX_TEST_BIN   := $(BUILD_DIR)/test_cpp

.PHONY: help all build configure test valgrind diagnose clean

help:
	@echo "project_A —— 可用目标："
	@echo "  make build      增量编译 C++ 扩展（首次会自动 cmake 配置）"
	@echo "  make configure  仅重新执行 cmake 配置"
	@echo "  make test       激活 .venv 并运行 python tests/test_main.py"
	@echo "  make valgrind   编译 tests/test_cpp.cpp 并用 valgrind 检测内存"
	@echo "  make diagnose   一键诊断：valgrind + Python 测试"
	@echo "  make clean      清理 $(BUILD_DIR)/ 下的构建产物"
	@echo "  make help       显示本帮助"

all: build test

# ---------------------------------------------------------------------
# 增量编译：若 build/Makefile 不存在（例如刚 clean 过），先自动 cmake 配置。
# 注意：配置阶段必须在 venv 中执行，否则会命中系统 python3（没有 pybind11）。
# ---------------------------------------------------------------------
build:
	@if [ ! -f $(BUILD_DIR)/Makefile ]; then \
		echo ">>> 未检测到 CMake 配置，自动执行 cmake ../scripts ..."; \
		$(VENV_ACTIVATE) && cd $(BUILD_DIR) && cmake ../scripts; \
	fi
	@$(MAKE) -C $(BUILD_DIR)

configure:
	@$(VENV_ACTIVATE) && cd $(BUILD_DIR) && cmake ../scripts

# ---------------------------------------------------------------------
# Python 测试
# ---------------------------------------------------------------------
test:
	@cd $(PROJECT_ROOT) && $(VENV_ACTIVATE) && python tests/test_main.py

# ---------------------------------------------------------------------
# valgrind 内存检测（复用 diagnose.sh，保证逻辑只有一份）
# ---------------------------------------------------------------------
valgrind:
	@bash diagnose.sh --only valgrind

diagnose:
	@bash diagnose.sh

# ---------------------------------------------------------------------
# 清理构建产物（保留 build/ 目录本身）
# ---------------------------------------------------------------------
clean:
	@if [ -f $(BUILD_DIR)/Makefile ]; then \
		$(MAKE) -C $(BUILD_DIR) clean >/dev/null 2>&1 || true; \
	fi
	@rm -rf $(BUILD_DIR)/*
	@echo ">>> 已清理 $(BUILD_DIR)/ 下的构建产物（下次 make build 会自动重新配置）"

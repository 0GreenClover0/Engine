# Serialization Guide

## Overview

### Script Arguments

- **-pv --pick_vars**: Allows selection of variables to be serialized.
- **-pf --pick_files**: Allows selection of files to be serialized.

### Usage of Directives

After adding `#include "Serialization.h"`, you can control serialization behavior using the following directives:

- **NON_SERIALIZED**: 
  - Placed before a variable, this directive ensures the variable is not serialized.
  - Placed before a class, this directive ensures the class is not serialized.

## Requirements for Serializable Components

Every component that needs to be fully serialized should meet the following criteria:

- Must be a descendant of the `Component` class.
- Must have a `create()` method.
- All access qualifiers should begin with `public` before other qualifiers.
- All serializable variables should be declared in the `public` section.
- Variables should be formatted in the following way: `type variable = value;`, where the value can include uppercase and lowercase letters, digits, and symbols like `."{}`.

## Example

```cpp
#include "Component.h"
#include "Serialization.h"

class MyComponent : public Component 
{
public:
	static std::shared_ptr<MyComponent> create();
	int my_public_variable = 42; // Will be serialized
	NON_SERIALIZED
	int my_public_not_serialized_variable = 43; // Will NOT be serialized

private:
	int m_my_private_variable = 44; // Will NOT be serialized
};


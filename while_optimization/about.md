Replacing

```
while (cond) {
	statement;
}
```

with

```
if (cond) {
	do {
		statement;
	while (cond);
}
```

is useless in llvm

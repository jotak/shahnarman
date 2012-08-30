cost = {0,0,0,4}
icon = "spl_hellhounds"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Chiens des enfers"
	else
		return "Hell hounds"
	end
end

function getDescription()
	desc = getUnitDescription("HellHounds")
	if language == "french" then
		return "Invoque des chiens des enfers\n"..desc
	else
		return "Summons hell hounds\n"..desc
	end
end

function onCast()
	selectTarget("tile", "inrange")
end

function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "HellHounds")
end

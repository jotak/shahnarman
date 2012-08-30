cost = {0,1,4,0}
icon = "canard"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Invoquer boss 1"
	else
		return "Summon boss 1"
	end
end

function getDescription()
	if language == "french" then
		return ""
	else
		return ""
	end
end

function onCast()
	selectTarget("tile", "inrange")
end

-- this function receives map coords. as parameter, in a space-separated string
function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "Extension1_Boss")
end
